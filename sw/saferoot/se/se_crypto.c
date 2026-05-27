// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// se_crypto.c — Crypto operations for Secure Element
// Wraps HAL AES/HMAC drivers and CSRNG hardware.

#include "se_crypto.h"
#include "se_keystore.h"
#include "../hal/hal.h"
#include "../hal/aes_drv.h"
#include "../hal/hmac_drv.h"

// Bare-metal stubs
#ifndef NULL
#define NULL ((void*)0)
#endif
extern void *memcpy(void *dest, const void *src, unsigned int n);
extern void *memset(void *s, int c, unsigned int n);

// ---------------------------------------------------------------------------
// CSRNG hardware registers (OpenTitan CSRNG IP at 0x40030000)
// ---------------------------------------------------------------------------
#define CSRNG_BASE              0x40030000U
#define CSRNG_CMD_REQ           (CSRNG_BASE + 0x14U)
#define CSRNG_GENBITS_VLD       (CSRNG_BASE + 0x1CU)
#define CSRNG_GENBITS           (CSRNG_BASE + 0x20U)
#define CSRNG_INT_STATE         (CSRNG_BASE + 0x24U)

// CSRNG application commands
#define CSRNG_CMD_INSTANTIATE   0x01U
#define CSRNG_CMD_GENERATE      0x03U

// ---------------------------------------------------------------------------
// se_get_random — Read random bytes from CSRNG hardware
// ---------------------------------------------------------------------------
uint16_t se_get_random(uint8_t *buf, uint32_t len) {
    if (buf == NULL || len == 0) {
        return SW_WRONG_DATA;
    }

    // Issue GENERATE command: acmd=GENERATE, clen=0, glen=ceil(len/16) blocks
    uint32_t num_blocks = (len + 15) / 16;
    uint32_t cmd_word = (num_blocks << 12) | CSRNG_CMD_GENERATE;
    REG32_WRITE(CSRNG_CMD_REQ, cmd_word);

    uint32_t offset = 0;
    for (uint32_t blk = 0; blk < num_blocks; blk++) {
        // Wait for genbits_vld
        uint32_t timeout = HAL_TIMEOUT_CYCLES;
        while ((REG32_READ(CSRNG_GENBITS_VLD) & 0x01U) == 0) {
            if (--timeout == 0) {
                return SW_INTERNAL_ERROR;
            }
        }

        // Read 4 words (128 bits) of random data
        for (uint32_t w = 0; w < 4; w++) {
            uint32_t rnd = REG32_READ(CSRNG_GENBITS);
            for (uint32_t b = 0; b < 4 && offset < len; b++, offset++) {
                buf[offset] = (uint8_t)(rnd >> (b * 8));
            }
        }
    }

    return SW_OK;
}

// ---------------------------------------------------------------------------
// se_aes_encrypt — AES-256-ECB encrypt using key from slot
// ---------------------------------------------------------------------------
uint16_t se_aes_encrypt(uint8_t slot, const uint8_t *in,
                        uint8_t *out, uint32_t len) {
    if (in == NULL || out == NULL || len == 0 || (len % 16) != 0) {
        return SW_WRONG_LENGTH;
    }

    // Retrieve key from store
    se_key_t key;
    uint16_t sw = keystore_get(slot, &key);
    if (sw != SW_OK) {
        return sw;
    }
    if (key.type != kKeyTypeAes256) {
        return SW_WRONG_DATA;
    }

    // Configure AES hardware for encryption
    hal_status_t st = aes_init(kAesEncrypt, kAesModeEcb, key.key, NULL);
    if (st != kHalOk) {
        return SW_INTERNAL_ERROR;
    }

    // Process each 16-byte block
    for (uint32_t off = 0; off < len; off += 16) {
        uint32_t block_in[4];
        uint32_t block_out[4];
        memcpy(block_in, in + off, 16);

        st = aes_process_block(block_in, block_out);
        if (st != kHalOk) {
            aes_clear();
            return SW_INTERNAL_ERROR;
        }
        memcpy(out + off, block_out, 16);
    }

    aes_clear();
    return SW_OK;
}

// ---------------------------------------------------------------------------
// se_aes_decrypt — AES-256-ECB decrypt using key from slot
// ---------------------------------------------------------------------------
uint16_t se_aes_decrypt(uint8_t slot, const uint8_t *in,
                        uint8_t *out, uint32_t len) {
    if (in == NULL || out == NULL || len == 0 || (len % 16) != 0) {
        return SW_WRONG_LENGTH;
    }

    se_key_t key;
    uint16_t sw = keystore_get(slot, &key);
    if (sw != SW_OK) {
        return sw;
    }
    if (key.type != kKeyTypeAes256) {
        return SW_WRONG_DATA;
    }

    hal_status_t st = aes_init(kAesDecrypt, kAesModeEcb, key.key, NULL);
    if (st != kHalOk) {
        return SW_INTERNAL_ERROR;
    }

    for (uint32_t off = 0; off < len; off += 16) {
        uint32_t block_in[4];
        uint32_t block_out[4];
        memcpy(block_in, in + off, 16);

        st = aes_process_block(block_in, block_out);
        if (st != kHalOk) {
            aes_clear();
            return SW_INTERNAL_ERROR;
        }
        memcpy(out + off, block_out, 16);
    }

    aes_clear();
    return SW_OK;
}

// ---------------------------------------------------------------------------
// se_sign — ECDSA-P256 sign (STUB)
// TODO: Implement using OTBN coprocessor or software bignum library.
//       OTBN program: p256_ecdsa_sign.s
// ---------------------------------------------------------------------------
uint16_t se_sign(uint8_t slot, const uint8_t *hash, uint8_t *sig) {
    se_key_t key;
    uint16_t sw = keystore_get(slot, &key);
    if (sw != SW_OK) {
        return sw;
    }
    if (key.type != kKeyTypeEcdsaP256) {
        return SW_WRONG_DATA;
    }

    (void)hash;
    (void)sig;

    // TODO: Load OTBN program, set key/hash, trigger, read r||s
    return SW_INTERNAL_ERROR;  // Not yet implemented
}

// ---------------------------------------------------------------------------
// se_verify — ECDSA-P256 verify (STUB)
// TODO: Implement using OTBN coprocessor or software bignum library.
//       OTBN program: p256_ecdsa_verify.s
// ---------------------------------------------------------------------------
uint16_t se_verify(uint8_t slot, const uint8_t *hash, const uint8_t *sig) {
    se_key_t key;
    uint16_t sw = keystore_get(slot, &key);
    if (sw != SW_OK) {
        return sw;
    }
    if (key.type != kKeyTypeEcdsaP256) {
        return SW_WRONG_DATA;
    }

    (void)hash;
    (void)sig;

    // TODO: Load OTBN program, set pubkey/hash/sig, trigger, check result
    return SW_INTERNAL_ERROR;  // Not yet implemented
}
