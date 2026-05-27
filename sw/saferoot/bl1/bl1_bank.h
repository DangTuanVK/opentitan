// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// bl1_bank.h — A/B firmware bank management for BL1
// Supports dual-bank firmware update with fallback.

#ifndef SAFEROOT_BL1_BANK_H_
#define SAFEROOT_BL1_BANK_H_

#include <stdint.h>
#include <stdbool.h>
#include "../bl0/bl0.h"
#include "bl1.h"

// ---------------------------------------------------------------------------
// Bank base addresses in Flash
// ---------------------------------------------------------------------------
#define BANK_A_BASE             0x20010000U
#define BANK_B_BASE             0x20050000U

// Number of banks
#define BANK_COUNT              2U

// OTP address for bank status (which bank is marked good)
#define OTP_BANK_STATUS_ADDR    0x0308U

// Bank status flags stored in OTP/Flash metadata
#define BANK_FLAG_VALID         0x0001U
#define BANK_FLAG_GOOD          0x0002U

// ---------------------------------------------------------------------------
// Bank identifier
// ---------------------------------------------------------------------------
typedef enum {
    kBankA = 0,
    kBankB = 1,
} bank_id_t;

// ---------------------------------------------------------------------------
// Bank info structure
// ---------------------------------------------------------------------------
typedef struct {
    bank_id_t       id;
    uint32_t        base_addr;
    bool            valid;          // Header parsed successfully
    uint32_t        version_major;
    uint32_t        version_minor;
    uint32_t        security_version;
} bank_info_t;

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

// Select the best bank: read both headers, pick valid + newer version.
// Returns kBl1Ok on success and fills *selected with the chosen bank.
// Returns kBl1ErrNoBankValid if neither bank has a valid header.
bl1_error_t bank_select(bank_info_t *selected);

// Return the base address of the active (selected) bank.
uint32_t bank_get_active(void);

// Mark the currently active bank as verified-good in OTP metadata.
bl1_error_t bank_mark_good(bank_id_t id);

#endif  // SAFEROOT_BL1_BANK_H_
