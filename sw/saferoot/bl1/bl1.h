// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// bl1.h — BL1 (Owner Firmware) Secure Boot API header
// Second mutable boot stage: called by BL0 after DICE CDI_0 derivation.
// Runs from Flash, verifies Application firmware before jumping to it.

#ifndef SAFEROOT_BL1_H_
#define SAFEROOT_BL1_H_

#include <stdint.h>
#include <stdbool.h>
#include "../bl0/bl0.h"

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

// Application firmware location in Flash (Bank A, after 32KB BL0 + 32KB BL1)
#define APP_FLASH_BASE          0x20010000U

// Maximum Application image size: 256KB per bank
#define APP_MAX_SIZE            (256U * 1024U)

// BL1 CDI derivation descriptor
#define BL1_CDI_DESCRIPTOR      "SafeRoot.BL1.CDI_1"
#define BL1_CDI_DESCRIPTOR_LEN  18U

// OTP address for BL1-level anti-rollback SVN (OwnerSwCfg partition)
#define OTP_BL1_SVN_ADDR        0x0304U

// Maximum boot retry count before halting
#define BL1_MAX_RETRIES         3U

// Watchdog bark/bite timeouts (AON timer cycles, ~200kHz)
// Bark at 5 seconds, bite at 8 seconds
#define BL1_WDT_BARK_TIMEOUT    1000000U
#define BL1_WDT_BITE_TIMEOUT    1600000U

// ---------------------------------------------------------------------------
// Owner key (public key for Application verification)
// ---------------------------------------------------------------------------
typedef struct {
    uint32_t key_x[SHA256_DIGEST_WORDS];    // ECDSA-P256 public key X
    uint32_t key_y[SHA256_DIGEST_WORDS];    // ECDSA-P256 public key Y
    uint32_t key_id;                        // Key identifier
} owner_key_t;

// ---------------------------------------------------------------------------
// Error codes
// ---------------------------------------------------------------------------
typedef enum {
    kBl1Ok              = 0,    // Success
    kBl1ErrBadMagic     = 1,    // Image header magic mismatch
    kBl1ErrBadSize      = 2,    // Image size out of range
    kBl1ErrHashMismatch = 3,    // SHA-256 hash mismatch
    kBl1ErrSigVerify    = 4,    // ECDSA signature verification failed
    kBl1ErrAntiRollback = 5,    // SVN anti-rollback check failed
    kBl1ErrDice         = 6,    // DICE CDI_1 derivation error
    kBl1ErrFlashRead    = 7,    // Flash read error
    kBl1ErrOtpRead      = 8,    // OTP read error
    kBl1ErrNoBankValid  = 9,    // No valid firmware bank found
    kBl1ErrRetryExhaust = 10,   // All retries exhausted
} bl1_error_t;

// ---------------------------------------------------------------------------
// BL1 entry point — called by BL0 after CDI_0 derivation
// ---------------------------------------------------------------------------
void bl1_main(const dice_cdi_t *cdi_0) __attribute__((noreturn));

#endif  // SAFEROOT_BL1_H_
