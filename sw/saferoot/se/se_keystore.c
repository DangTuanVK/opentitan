// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// se_keystore.c — Key management for Secure Element
// Keys live in SRAM (volatile) and can be persisted to Flash (encrypted).

#include "se_keystore.h"
#include "se_crypto.h"
#include "../hal/hal.h"
#include "../hal/aes_drv.h"
#include "../hal/otp_drv.h"
#include "../hal/flash_drv.h"

// Bare-metal stubs
#ifndef NULL
#define NULL ((void*)0)
#endif
extern void *memcpy(void *dest, const void *src, unsigned int n);
extern void *memset(void *s, int c, unsigned int n);

// ---------------------------------------------------------------------------
// Key store layout
// ---------------------------------------------------------------------------

// Flash address where encrypted key slots are stored (bank 1, page 240+)
#define KEYSTORE_FLASH_BASE  (SAFEROOT_FLASH_MEM_BASE + \
                              (FLASH_BANK_PAGES * FLASH_PAGE_SIZE) + \
                              (240U * FLASH_PAGE_SIZE))

// OTP address of the 256-bit key-wrapping key (in Secret2 partition)
#define OTP_WRAP_KEY_ADDR    0x0900U

// Size of one serialized key slot in flash (words)
#define KEYSTORE_SLOT_WORDS  24  // id(1) + type(1) + flags(1) + pad(1) +
                                 // key[8] + pub_x[8] + pub_y[8] = 28,
                                 // rounded: 24 words for core fields

// In-memory key store
static se_key_t g_key_slots[SE_MAX_KEY_SLOTS];

// OTP-derived wrapping key for flash persistence
static uint32_t g_wrap_key[8];

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

// Zeroize memory (avoid compiler optimization removing it)
static void secure_zeroize(void *ptr, uint32_t len) {
    volatile uint8_t *p = (volatile uint8_t *)ptr;
    while (len--) {
        *p++ = 0;
    }
}

// Load the wrapping key from OTP Secret2 partition
static void load_wrap_key(void) {
    for (uint32_t i = 0; i < 4; i++) {
        otp_read_word64(OTP_WRAP_KEY_ADDR + (i * 8),
                        &g_wrap_key[i * 2],
                        &g_wrap_key[i * 2 + 1]);
    }
}

// ---------------------------------------------------------------------------
// keystore_init — Load persistent keys from Flash into SRAM
// ---------------------------------------------------------------------------
void keystore_init(void) {
    // Clear all slots
    memset(g_key_slots, 0, sizeof(g_key_slots));

    // Load wrapping key from OTP
    otp_init();
    load_wrap_key();

    // Initialize flash controller
    flash_init();

    // Try to load each slot from flash
    for (uint8_t i = 0; i < SE_MAX_KEY_SLOTS; i++) {
        uint32_t flash_addr = KEYSTORE_FLASH_BASE + (i * KEYSTORE_SLOT_WORDS * 4);
        uint32_t buf[KEYSTORE_SLOT_WORDS];

        if (flash_read(flash_addr, buf, KEYSTORE_SLOT_WORDS) != kHalOk) {
            continue;
        }

        // Check magic/valid marker: first word must have ACTIVE flag set
        uint8_t flags = (uint8_t)(buf[2] & 0xFF);
        if ((flags & SE_KEY_FLAG_ACTIVE) == 0) {
            continue;  // Empty slot in flash
        }

        // Decrypt key material using wrapping key (AES-256-ECB)
        // Key data starts at word index 3, spans 8 words (32 bytes)
        uint32_t dec_key[8];
        // Set up AES with wrap key for decryption
        if (aes_init(kAesDecrypt, kAesModeEcb, g_wrap_key, NULL) == kHalOk) {
            // Decrypt two 16-byte blocks of key material
            aes_process_block(&buf[3], &dec_key[0]);
            aes_process_block(&buf[7], &dec_key[4]);
            aes_clear();

            g_key_slots[i].id    = (uint8_t)(buf[0] & 0xFF);
            g_key_slots[i].type  = (se_key_type_t)(buf[1] & 0xFF);
            g_key_slots[i].flags = flags;
            memcpy(g_key_slots[i].key, dec_key, sizeof(dec_key));

            // Public key (not encrypted) — words 11..26
            if (KEYSTORE_SLOT_WORDS >= 24) {
                memcpy(g_key_slots[i].pub_x, &buf[11], 32);
                memcpy(g_key_slots[i].pub_y, &buf[19], 32);
            }
        }

        secure_zeroize(dec_key, sizeof(dec_key));
        secure_zeroize(buf, sizeof(buf));
    }
}

// ---------------------------------------------------------------------------
// keystore_generate — Generate a new key in the specified slot
// ---------------------------------------------------------------------------
uint16_t keystore_generate(uint8_t slot, se_key_type_t type) {
    if (slot >= SE_MAX_KEY_SLOTS) {
        return SW_WRONG_DATA;
    }

    // Zeroize the slot first
    secure_zeroize(&g_key_slots[slot], sizeof(se_key_t));

    g_key_slots[slot].id   = slot;
    g_key_slots[slot].type = type;

    switch (type) {
        case kKeyTypeAes256:
        case kKeyTypeHmac256: {
            // Generate 256-bit random symmetric key
            uint16_t sw = se_get_random((uint8_t *)g_key_slots[slot].key, 32);
            if (sw != SW_OK) {
                secure_zeroize(&g_key_slots[slot], sizeof(se_key_t));
                return sw;
            }
            g_key_slots[slot].flags = SE_KEY_FLAG_ACTIVE;
            break;
        }

        case kKeyTypeEcdsaP256:
            // TODO: Generate P-256 key pair via OTBN coprocessor.
            // For now, generate random private key; public key = placeholder.
            {
                uint16_t sw = se_get_random(
                    (uint8_t *)g_key_slots[slot].key, 32);
                if (sw != SW_OK) {
                    secure_zeroize(&g_key_slots[slot], sizeof(se_key_t));
                    return sw;
                }
                // TODO: Compute public key Q = d * G using OTBN
                memset(g_key_slots[slot].pub_x, 0, 32);
                memset(g_key_slots[slot].pub_y, 0, 32);
                g_key_slots[slot].flags = SE_KEY_FLAG_ACTIVE;
            }
            break;

        default:
            return SW_WRONG_DATA;
    }

    return SW_OK;
}

// ---------------------------------------------------------------------------
// keystore_get — Retrieve key from slot (internal use only)
// ---------------------------------------------------------------------------
uint16_t keystore_get(uint8_t slot, se_key_t *key) {
    if (slot >= SE_MAX_KEY_SLOTS || key == NULL) {
        return SW_WRONG_DATA;
    }
    if ((g_key_slots[slot].flags & SE_KEY_FLAG_ACTIVE) == 0) {
        return SW_KEY_NOT_FOUND;
    }

    memcpy(key, &g_key_slots[slot], sizeof(se_key_t));
    return SW_OK;
}

// ---------------------------------------------------------------------------
// keystore_delete — Securely zeroize a key slot
// ---------------------------------------------------------------------------
uint16_t keystore_delete(uint8_t slot) {
    if (slot >= SE_MAX_KEY_SLOTS) {
        return SW_WRONG_DATA;
    }
    if ((g_key_slots[slot].flags & SE_KEY_FLAG_ACTIVE) == 0) {
        return SW_KEY_NOT_FOUND;
    }

    secure_zeroize(&g_key_slots[slot], sizeof(se_key_t));
    return SW_OK;
}

// ---------------------------------------------------------------------------
// keystore_persist — Save key slot to flash (encrypted with wrapping key)
// ---------------------------------------------------------------------------
uint16_t keystore_persist(uint8_t slot) {
    if (slot >= SE_MAX_KEY_SLOTS) {
        return SW_WRONG_DATA;
    }
    if ((g_key_slots[slot].flags & SE_KEY_FLAG_ACTIVE) == 0) {
        return SW_KEY_NOT_FOUND;
    }

    uint32_t buf[KEYSTORE_SLOT_WORDS];
    memset(buf, 0, sizeof(buf));

    buf[0] = g_key_slots[slot].id;
    buf[1] = (uint32_t)g_key_slots[slot].type;
    buf[2] = g_key_slots[slot].flags | SE_KEY_FLAG_PERSISTENT;

    // Encrypt key material with wrapping key (AES-256-ECB, two blocks)
    if (aes_init(kAesEncrypt, kAesModeEcb, g_wrap_key, NULL) != kHalOk) {
        return SW_INTERNAL_ERROR;
    }
    aes_process_block(g_key_slots[slot].key, &buf[3]);
    aes_process_block(&g_key_slots[slot].key[4], &buf[7]);
    aes_clear();

    // Public key stored in plaintext
    memcpy(&buf[11], g_key_slots[slot].pub_x, 32);
    memcpy(&buf[19], g_key_slots[slot].pub_y, 32);

    // Erase flash page and write
    uint32_t flash_addr = KEYSTORE_FLASH_BASE + (slot * KEYSTORE_SLOT_WORDS * 4);
    if (flash_erase_page(flash_addr) != kHalOk) {
        return SW_INTERNAL_ERROR;
    }
    if (flash_write(flash_addr, buf, KEYSTORE_SLOT_WORDS) != kHalOk) {
        return SW_INTERNAL_ERROR;
    }

    // Mark as persistent in SRAM copy
    g_key_slots[slot].flags |= SE_KEY_FLAG_PERSISTENT;

    secure_zeroize(buf, sizeof(buf));
    return SW_OK;
}
