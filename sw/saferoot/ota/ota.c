// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// ota.c -- OTA Firmware Update Manager implementation
// Power-fail safe: metadata is written atomically as the last step.
// Anti-rollback: image SVN must be >= OTP minimum SVN.

#include "ota.h"
#include "../hal/hal.h"
#include "../hal/flash_drv.h"
#include "../hal/hmac_drv.h"
#include "../hal/otp_drv.h"
#include "../hal/uart_drv.h"
#include "../bl0/bl0.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

extern void *memcpy(void *dst, const void *src, unsigned int n);
extern void *memset(void *s, int c, unsigned int n);

// ---------------------------------------------------------------------------
// Module state
// ---------------------------------------------------------------------------
static ota_state_t    g_state;
static ota_metadata_t g_meta;           // Current active bank metadata
static uint32_t       g_active_bank;    // 0 = A, 1 = B
static uint32_t       g_inactive_base;  // Base address of inactive bank
static uint32_t       g_active_base;    // Base address of active bank
static uint32_t       g_pending_size;   // Expected image size for current OTA
static uint32_t       g_pending_ver;    // Expected image version for current OTA

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static uint32_t bank_base(uint32_t bank) {
    return (bank == 0) ? OTA_BANK_A_BASE : OTA_BANK_B_BASE;
}

static uint32_t meta_addr(uint32_t base) {
    return base + OTA_META_OFFSET;
}

static ota_error_t read_metadata(uint32_t base, ota_metadata_t *m) {
    uint32_t addr = meta_addr(base);
    hal_status_t st = flash_read(addr, (uint32_t *)m,
                                 sizeof(ota_metadata_t) / 4);
    if (st != kHalOk) {
        return kOtaErrFlash;
    }
    return kOtaOk;
}

static ota_error_t write_metadata(uint32_t base, const ota_metadata_t *m) {
    uint32_t addr = meta_addr(base);

    // Erase the metadata page first
    hal_status_t st = flash_erase_page(addr);
    if (st != kHalOk) {
        return kOtaErrFlash;
    }

    // Program metadata
    st = flash_write(addr, (const uint32_t *)m,
                     sizeof(ota_metadata_t) / 4);
    if (st != kHalOk) {
        return kOtaErrFlash;
    }

    HAL_BARRIER();
    return kOtaOk;
}

static ota_error_t check_anti_rollback(uint32_t svn) {
    uint32_t min_svn = 0;
    hal_status_t st = otp_read_word(OTA_OTP_MIN_SVN_ADDR, &min_svn);
    if (st != kHalOk) {
        return kOtaErrOtp;
    }
    if (svn < min_svn) {
        return kOtaErrAntiRollback;
    }
    return kOtaOk;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

ota_error_t ota_init(void) {
    ota_metadata_t meta_a, meta_b;
    bool a_valid = false;
    bool b_valid = false;

    g_state = kOtaStateIdle;

    // Read metadata from both banks
    if (read_metadata(OTA_BANK_A_BASE, &meta_a) == kOtaOk &&
        meta_a.magic == OTA_META_MAGIC && meta_a.valid == 1) {
        a_valid = true;
    }
    if (read_metadata(OTA_BANK_B_BASE, &meta_b) == kOtaOk &&
        meta_b.magic == OTA_META_MAGIC && meta_b.valid == 1) {
        b_valid = true;
    }

    // Determine active bank: prefer the one marked active; if both valid,
    // pick the one with higher version; default to A.
    if (a_valid && b_valid) {
        if (meta_b.active_bank == 1 && meta_a.active_bank == 0) {
            g_active_bank = 1;
            memcpy(&g_meta, &meta_b, sizeof(ota_metadata_t));
        } else {
            g_active_bank = 0;
            memcpy(&g_meta, &meta_a, sizeof(ota_metadata_t));
        }
    } else if (b_valid) {
        g_active_bank = 1;
        memcpy(&g_meta, &meta_b, sizeof(ota_metadata_t));
    } else {
        // Default to A (even if neither is valid -- first boot)
        g_active_bank = 0;
        if (a_valid) {
            memcpy(&g_meta, &meta_a, sizeof(ota_metadata_t));
        } else {
            memset(&g_meta, 0, sizeof(ota_metadata_t));
            g_meta.magic = OTA_META_MAGIC;
            g_meta.active_bank = 0;
            g_meta.valid = 1;
        }
    }

    g_active_base   = bank_base(g_active_bank);
    g_inactive_base = bank_base(g_active_bank ^ 1);

    uart_puts("[OTA] init: active bank ");
    uart_put_hex(g_active_bank);
    uart_puts("\r\n");

    return kOtaOk;
}

ota_error_t ota_begin(uint32_t image_size, uint32_t image_version) {
    if (g_state != kOtaStateIdle) {
        return kOtaErrBadState;
    }

    // Validate image size
    if (image_size == 0 || image_size > OTA_MAX_IMAGE_SIZE) {
        return kOtaErrBadSize;
    }

    // Anti-rollback: new version must be >= OTP minimum SVN
    ota_error_t err = check_anti_rollback(image_version);
    if (err != kOtaOk) {
        g_state = kOtaStateError;
        return err;
    }

    // Version must be >= current active version
    if (image_version < g_meta.image_version) {
        g_state = kOtaStateError;
        return kOtaErrAntiRollback;
    }

    g_pending_size = image_size;
    g_pending_ver  = image_version;

    uart_puts("[OTA] erasing inactive bank...\r\n");

    // Erase all pages of the inactive bank
    uint32_t pages = (OTA_BANK_SIZE + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
    for (uint32_t i = 0; i < pages; i++) {
        hal_status_t st = flash_erase_page(g_inactive_base + i * FLASH_PAGE_SIZE);
        if (st != kHalOk) {
            g_state = kOtaStateError;
            return kOtaErrFlash;
        }
    }

    g_state = kOtaStateReceiving;
    uart_puts("[OTA] begin: ready to receive\r\n");
    return kOtaOk;
}

ota_error_t ota_write_chunk(const uint8_t *data, uint32_t offset, uint32_t len) {
    if (g_state != kOtaStateReceiving) {
        return kOtaErrBadState;
    }
    if (data == NULL || len == 0) {
        return kOtaErrBadArg;
    }
    if (offset + len > OTA_MAX_IMAGE_SIZE) {
        return kOtaErrBadSize;
    }
    // len must be word-aligned for flash_write
    if ((len & 3) != 0 || (offset & 3) != 0) {
        return kOtaErrBadArg;
    }

    uint32_t addr = g_inactive_base + offset;
    hal_status_t st = flash_write(addr, (const uint32_t *)data, len / 4);
    if (st != kHalOk) {
        g_state = kOtaStateError;
        return kOtaErrFlash;
    }

    return kOtaOk;
}

ota_error_t ota_finish(void) {
    if (g_state != kOtaStateReceiving) {
        return kOtaErrBadState;
    }

    g_state = kOtaStateVerifying;
    uart_puts("[OTA] verifying image hash...\r\n");

    // Read image header from the inactive bank
    fw_image_header_t hdr;
    hal_status_t st = flash_read(g_inactive_base, (uint32_t *)&hdr,
                                 sizeof(fw_image_header_t) / 4);
    if (st != kHalOk) {
        g_state = kOtaStateError;
        return kOtaErrFlash;
    }

    // Validate header magic
    if (hdr.magic != FW_IMAGE_MAGIC) {
        g_state = kOtaStateError;
        return kOtaErrBadMagic;
    }

    // Validate payload size matches what was declared in ota_begin
    if (hdr.image_size > g_pending_size ||
        hdr.image_size > OTA_MAX_IMAGE_SIZE - sizeof(fw_image_header_t)) {
        g_state = kOtaStateError;
        return kOtaErrBadSize;
    }

    // Compute SHA-256 over payload (after header)
    uint32_t payload_addr = g_inactive_base + sizeof(fw_image_header_t);
    uint32_t digest[SHA256_DIGEST_WORDS];

    hmac_init(NULL);  // SHA-256 mode, no HMAC key

    // Hash in page-sized chunks to avoid large RAM buffers
    uint32_t remaining = hdr.image_size;
    uint32_t chunk_addr = payload_addr;
    uint32_t buf[FLASH_PAGE_SIZE / 4];

    while (remaining > 0) {
        uint32_t chunk = (remaining > FLASH_PAGE_SIZE) ? FLASH_PAGE_SIZE : remaining;
        uint32_t words = (chunk + 3) / 4;

        st = flash_read(chunk_addr, buf, words);
        if (st != kHalOk) {
            g_state = kOtaStateError;
            return kOtaErrFlash;
        }

        hmac_update(buf, chunk);
        chunk_addr += chunk;
        remaining  -= chunk;
    }

    hmac_final(digest);

    // Compare computed hash with header hash
    for (uint32_t i = 0; i < SHA256_DIGEST_WORDS; i++) {
        if (digest[i] != hdr.image_hash[i]) {
            g_state = kOtaStateError;
            uart_puts("[OTA] HASH MISMATCH\r\n");
            return kOtaErrHashMismatch;
        }
    }

    // Write metadata for inactive bank (not yet marked active)
    g_state = kOtaStateProgramming;

    ota_metadata_t new_meta;
    memset(&new_meta, 0, sizeof(ota_metadata_t));
    new_meta.magic         = OTA_META_MAGIC;
    new_meta.active_bank   = g_active_bank;  // Still current active
    new_meta.boot_count    = g_meta.boot_count;
    new_meta.image_version = hdr.security_version;
    new_meta.last_good_version = g_meta.last_good_version;
    new_meta.valid         = 1;

    ota_error_t err = write_metadata(g_inactive_base, &new_meta);
    if (err != kOtaOk) {
        g_state = kOtaStateError;
        return err;
    }

    g_state = kOtaStateDone;
    uart_puts("[OTA] image verified OK\r\n");
    return kOtaOk;
}

ota_error_t ota_swap(void) {
    if (g_state != kOtaStateDone) {
        return kOtaErrBadState;
    }

    g_state = kOtaStateSwapping;
    uart_puts("[OTA] swapping banks...\r\n");

    uint32_t new_active = g_active_bank ^ 1;

    // Update metadata on the NEW active bank -- this is the atomic commit point.
    ota_metadata_t swap_meta;
    memset(&swap_meta, 0, sizeof(ota_metadata_t));
    swap_meta.magic              = OTA_META_MAGIC;
    swap_meta.active_bank        = new_active;
    swap_meta.boot_count         = 0;
    swap_meta.image_version      = g_pending_ver;
    swap_meta.last_good_version  = g_meta.image_version;
    swap_meta.valid              = 1;

    ota_error_t err = write_metadata(bank_base(new_active), &swap_meta);
    if (err != kOtaOk) {
        g_state = kOtaStateError;
        return err;
    }

    // Invalidate old active bank metadata so it becomes the fallback
    ota_metadata_t old_meta;
    memcpy(&old_meta, &g_meta, sizeof(ota_metadata_t));
    old_meta.active_bank = new_active;  // Point to new active
    write_metadata(g_active_base, &old_meta);

    HAL_BARRIER();

    g_active_bank   = new_active;
    g_active_base   = bank_base(new_active);
    g_inactive_base = bank_base(new_active ^ 1);
    memcpy(&g_meta, &swap_meta, sizeof(ota_metadata_t));

    g_state = kOtaStateIdle;
    uart_puts("[OTA] swap complete. Reboot to activate.\r\n");
    return kOtaOk;
}

ota_error_t ota_rollback(void) {
    // Rollback: swap active bank to the other one
    uint32_t rollback_bank = g_active_bank ^ 1;
    uint32_t rollback_base = bank_base(rollback_bank);

    // Read rollback bank metadata to ensure it has a valid image
    ota_metadata_t rb_meta;
    ota_error_t err = read_metadata(rollback_base, &rb_meta);
    if (err != kOtaOk || rb_meta.magic != OTA_META_MAGIC || rb_meta.valid != 1) {
        return kOtaErrNoInactive;
    }

    // Anti-rollback check on the rollback image
    err = check_anti_rollback(rb_meta.image_version);
    if (err != kOtaOk) {
        return err;
    }

    // Atomically mark rollback bank as active
    rb_meta.active_bank       = rollback_bank;
    rb_meta.last_good_version = g_meta.image_version;
    err = write_metadata(rollback_base, &rb_meta);
    if (err != kOtaOk) {
        return err;
    }

    g_active_bank   = rollback_bank;
    g_active_base   = rollback_base;
    g_inactive_base = bank_base(rollback_bank ^ 1);
    memcpy(&g_meta, &rb_meta, sizeof(ota_metadata_t));
    g_state = kOtaStateIdle;

    uart_puts("[OTA] rollback complete. Reboot to activate.\r\n");
    return kOtaOk;
}

ota_state_t ota_get_status(void) {
    return g_state;
}
