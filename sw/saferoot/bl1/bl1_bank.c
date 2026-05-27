// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// bl1_bank.c — A/B firmware bank selection and management
//
// SafeRoot supports two firmware banks (A and B) for resilient updates.
// Bank selection picks the valid bank with the higher version number.
// If both are equal, Bank A is preferred.

#include "bl1_bank.h"
#include "../hal/hal.h"
#include "../hal/flash_drv.h"
#include "../hal/otp_drv.h"
#include "../hal/uart_drv.h"

// ---------------------------------------------------------------------------
// Module state: active bank after selection
// ---------------------------------------------------------------------------
static uint32_t active_bank_base = 0U;

// ---------------------------------------------------------------------------
// Internal: read and validate a bank header
// ---------------------------------------------------------------------------
static bl1_error_t bank_read_header(uint32_t base_addr, bank_info_t *info)
{
    fw_image_header_t hdr;
    hal_status_t st;

    uint32_t header_words = sizeof(fw_image_header_t) / 4U;
    st = flash_read(base_addr, (uint32_t *)&hdr, header_words);
    if (st != kHalOk) {
        info->valid = false;
        return kBl1ErrFlashRead;
    }

    // Validate magic
    if (hdr.magic != FW_IMAGE_MAGIC) {
        info->valid = false;
        return kBl1ErrBadMagic;
    }

    // Validate size
    if (hdr.image_size == 0 || hdr.image_size > APP_MAX_SIZE) {
        info->valid = false;
        return kBl1ErrBadSize;
    }

    // Size must be word-aligned
    if ((hdr.image_size & 0x3U) != 0) {
        info->valid = false;
        return kBl1ErrBadSize;
    }

    info->valid = true;
    info->version_major = hdr.version_major;
    info->version_minor = hdr.version_minor;
    info->security_version = hdr.security_version;

    return kBl1Ok;
}

// ---------------------------------------------------------------------------
// Internal: compare two bank versions.
// Returns >0 if a is newer, <0 if b is newer, 0 if equal.
// ---------------------------------------------------------------------------
static int32_t bank_compare_version(const bank_info_t *a,
                                    const bank_info_t *b)
{
    if (a->version_major != b->version_major) {
        return (int32_t)(a->version_major - b->version_major);
    }
    if (a->version_minor != b->version_minor) {
        return (int32_t)(a->version_minor - b->version_minor);
    }
    // Equal versions — prefer Bank A (lower index)
    return 0;
}

// ---------------------------------------------------------------------------
// Public: select the best firmware bank
// ---------------------------------------------------------------------------
bl1_error_t bank_select(bank_info_t *selected)
{
    bank_info_t banks[BANK_COUNT];

    // Initialize bank metadata
    banks[0].id = kBankA;
    banks[0].base_addr = BANK_A_BASE;
    banks[0].valid = false;

    banks[1].id = kBankB;
    banks[1].base_addr = BANK_B_BASE;
    banks[1].valid = false;

    // Read both bank headers
    uart_puts("  Bank A (0x20010000): ");
    bl1_error_t err_a = bank_read_header(BANK_A_BASE, &banks[0]);
    if (banks[0].valid) {
        uart_puts("valid v");
        uart_put_hex(banks[0].version_major);
        uart_puts(".");
        uart_put_hex(banks[0].version_minor);
        uart_puts("\r\n");
    } else {
        uart_puts("invalid\r\n");
    }

    uart_puts("  Bank B (0x20050000): ");
    bl1_error_t err_b = bank_read_header(BANK_B_BASE, &banks[1]);
    if (banks[1].valid) {
        uart_puts("valid v");
        uart_put_hex(banks[1].version_major);
        uart_puts(".");
        uart_put_hex(banks[1].version_minor);
        uart_puts("\r\n");
    } else {
        uart_puts("invalid\r\n");
    }

    (void)err_a;
    (void)err_b;

    // Select the best bank
    if (banks[0].valid && banks[1].valid) {
        // Both valid — pick newer version
        int32_t cmp = bank_compare_version(&banks[0], &banks[1]);
        if (cmp >= 0) {
            *selected = banks[0];
            uart_puts("  Selected: Bank A (newer or equal)\r\n");
        } else {
            *selected = banks[1];
            uart_puts("  Selected: Bank B (newer)\r\n");
        }
    } else if (banks[0].valid) {
        *selected = banks[0];
        uart_puts("  Selected: Bank A (only valid)\r\n");
    } else if (banks[1].valid) {
        *selected = banks[1];
        uart_puts("  Selected: Bank B (only valid)\r\n");
    } else {
        uart_puts("  ERROR: No valid bank found!\r\n");
        return kBl1ErrNoBankValid;
    }

    active_bank_base = selected->base_addr;
    return kBl1Ok;
}

// ---------------------------------------------------------------------------
// Public: return the active bank base address
// ---------------------------------------------------------------------------
uint32_t bank_get_active(void)
{
    return active_bank_base;
}

// ---------------------------------------------------------------------------
// Public: mark a bank as verified-good in OTP metadata
// ---------------------------------------------------------------------------
bl1_error_t bank_mark_good(bank_id_t id)
{
    // Write bank-good flag to OTP.  The OTP word layout is:
    //   bits [15:0]  = Bank A flags
    //   bits [31:16] = Bank B flags
    uint32_t current = 0;
    hal_status_t st;

    st = otp_read_word(OTP_BANK_STATUS_ADDR, &current);
    if (st != kHalOk) {
        uart_puts("  OTP read bank status failed\r\n");
        return kBl1ErrOtpRead;
    }

    uint32_t new_val = current;
    if (id == kBankA) {
        new_val |= (BANK_FLAG_GOOD << 0);
    } else {
        new_val |= (BANK_FLAG_GOOD << 16);
    }

    // OTP is write-once; only write if the bit is not already set
    if (new_val != current) {
        st = otp_write_word(OTP_BANK_STATUS_ADDR, new_val);
        if (st != kHalOk) {
            uart_puts("  OTP write bank status failed\r\n");
            return kBl1ErrOtpRead;
        }
    }

    uart_puts("  Bank ");
    uart_puts(id == kBankA ? "A" : "B");
    uart_puts(" marked good\r\n");
    return kBl1Ok;
}
