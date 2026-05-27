// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// ota.h -- OTA (Over-The-Air) Firmware Update Manager for SafeRoot
// Handles secure firmware updates: receive via SPI, verify, program
// inactive flash bank, and swap on next boot.

#ifndef SAFEROOT_OTA_H_
#define SAFEROOT_OTA_H_

#include <stdint.h>
#include <stdbool.h>

// ---------------------------------------------------------------------------
// Flash bank layout
// ---------------------------------------------------------------------------
#define OTA_BANK_A_BASE       0x20010000U
#define OTA_BANK_B_BASE       0x20050000U
#define OTA_BANK_SIZE         (256U * 1024U)  // 256 KB per bank

// OTA metadata lives in the last page of each bank (2 KB page)
#define OTA_META_PAGE_SIZE    2048U
#define OTA_META_OFFSET       (OTA_BANK_SIZE - OTA_META_PAGE_SIZE)

// OTP address for minimum security version (anti-rollback)
#define OTA_OTP_MIN_SVN_ADDR  0x0300U

// Metadata magic: "OTAm" in little-endian
#define OTA_META_MAGIC        0x6D41544FU

// Maximum image size (bank size minus metadata page)
#define OTA_MAX_IMAGE_SIZE    (OTA_BANK_SIZE - OTA_META_PAGE_SIZE)

// ---------------------------------------------------------------------------
// OTA states
// ---------------------------------------------------------------------------
typedef enum {
    kOtaStateIdle        = 0,
    kOtaStateReceiving   = 1,
    kOtaStateVerifying   = 2,
    kOtaStateProgramming = 3,
    kOtaStateSwapping    = 4,
    kOtaStateDone        = 5,
    kOtaStateError       = 6,
} ota_state_t;

// ---------------------------------------------------------------------------
// OTA command codes (received from host SoC via SPI)
// ---------------------------------------------------------------------------
#define OTA_CMD_BEGIN     0x80U
#define OTA_CMD_DATA      0x81U
#define OTA_CMD_FINISH    0x82U
#define OTA_CMD_STATUS    0x83U
#define OTA_CMD_ROLLBACK  0x84U

// ---------------------------------------------------------------------------
// OTA error codes
// ---------------------------------------------------------------------------
typedef enum {
    kOtaOk              = 0,
    kOtaErrBadArg       = 1,
    kOtaErrFlash        = 2,
    kOtaErrHashMismatch = 3,
    kOtaErrAntiRollback = 4,
    kOtaErrBadMagic     = 5,
    kOtaErrBadSize      = 6,
    kOtaErrBadState     = 7,
    kOtaErrOtp          = 8,
    kOtaErrNoInactive   = 9,
} ota_error_t;

// ---------------------------------------------------------------------------
// OTA metadata (stored in last page of each flash bank)
// ---------------------------------------------------------------------------
typedef struct __attribute__((packed)) {
    uint32_t magic;              // OTA_META_MAGIC
    uint32_t active_bank;        // 0 = Bank A, 1 = Bank B
    uint32_t boot_count;         // Incremented each boot
    uint32_t last_good_version;  // SVN of last successfully booted image
    uint32_t image_version;      // SVN of image in this bank
    uint32_t valid;              // 1 = image verified OK
    uint32_t reserved[2];        // Future use
} ota_metadata_t;

// ---------------------------------------------------------------------------
// Function prototypes
// ---------------------------------------------------------------------------

// Initialize OTA subsystem: read metadata, determine active/inactive bank.
ota_error_t ota_init(void);

// Begin OTA update: validate version, erase inactive bank.
ota_error_t ota_begin(uint32_t image_size, uint32_t image_version);

// Write a chunk of firmware data to the inactive bank.
ota_error_t ota_write_chunk(const uint8_t *data, uint32_t offset, uint32_t len);

// Finish OTA: verify hash of written image against header.
ota_error_t ota_finish(void);

// Swap banks: mark inactive bank as new active (atomic metadata write).
ota_error_t ota_swap(void);

// Rollback: revert to previous bank.
ota_error_t ota_rollback(void);

// Return current OTA state.
ota_state_t ota_get_status(void);

#endif  // SAFEROOT_OTA_H_
