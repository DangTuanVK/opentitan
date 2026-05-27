// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// bl0.h — BL0 (ROM_EXT) Secure Boot API header
// First mutable boot stage: loaded from Flash by Boot ROM, verifies BL1.

#ifndef SAFEROOT_BL0_H_
#define SAFEROOT_BL0_H_

#include <stdint.h>
#include <stdbool.h>

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

// Firmware image magic number: "SRb1" in little-endian
#define FW_IMAGE_MAGIC          0x31625253U

// BL1 image location in flash (after BL0's 32KB region)
#define BL1_FLASH_BASE          0x20008000U

// Maximum BL1 image size: 224KB (flash bank 0 minus BL0 32KB)
#define BL1_MAX_SIZE            (224U * 1024U)

// SHA-256 digest size in 32-bit words
#define SHA256_DIGEST_WORDS     8U
#define SHA256_DIGEST_BYTES     32U

// ECDSA-P256 signature size in bytes
#define ECDSA_P256_SIG_BYTES    64U

// DICE CDI size in bytes
#define DICE_CDI_BYTES          32U

// OTP addresses for anti-rollback SVN
#define OTP_BL1_MIN_SVN_ADDR    0x0300U  // CreatorSwCfg partition

// OTP address for UDS (Unique Device Secret) — Secret2 partition
#define OTP_UDS_ADDR            0x0A00U

// Lifecycle controller base
#define LC_CTRL_BASE            0x40110000U

// Lifecycle state register offset
#define LC_CTRL_STATE_REG       0x001CU

// Lifecycle states (encoded values from lc_ctrl)
#define LC_STATE_DEV            0x15U
#define LC_STATE_PROD           0x16U
#define LC_STATE_PROD_END       0x17U
#define LC_STATE_RMA            0x18U

// Key Manager base + register offsets
#define KEYMGR_BASE             0x40100000U
#define KEYMGR_CFG_REG          0x0010U
#define KEYMGR_START_REG        0x0014U
#define KEYMGR_WORKING_STATE    0x0018U
#define KEYMGR_SW_BINDING_0     0x0040U

// GPIO pin for boot status LED
#define GPIO_BOOT_STATUS_PIN    0U

// ---------------------------------------------------------------------------
// Error codes
// ---------------------------------------------------------------------------
typedef enum {
    kBl0Ok              = 0,   // Success
    kBl0ErrLcState      = 1,   // Invalid lifecycle state
    kBl0ErrBadMagic     = 2,   // Image header magic mismatch
    kBl0ErrBadSize      = 3,   // Image size out of range
    kBl0ErrHashMismatch = 4,   // SHA-256 hash mismatch
    kBl0ErrSigVerify    = 5,   // ECDSA signature verification failed
    kBl0ErrAntiRollback = 6,   // SVN anti-rollback check failed
    kBl0ErrDice         = 7,   // DICE CDI derivation error
    kBl0ErrKeymgr       = 8,   // Key Manager advance failed
    kBl0ErrFlashRead    = 9,   // Flash read error
    kBl0ErrOtpRead      = 10,  // OTP read error
} bl0_error_t;

// ---------------------------------------------------------------------------
// Firmware image header (lives at the start of BL1 image in flash)
// ---------------------------------------------------------------------------
typedef struct __attribute__((packed)) {
    uint32_t magic;                             // FW_IMAGE_MAGIC
    uint32_t version_major;                     // Major version
    uint32_t version_minor;                     // Minor version
    uint32_t security_version;                  // Security Version Number (SVN)
    uint32_t image_size;                        // Payload size in bytes (excl. header)
    uint32_t entry_offset;                      // Entry point offset from image base
    uint32_t image_hash[SHA256_DIGEST_WORDS];   // SHA-256 of payload
    uint8_t  signature[ECDSA_P256_SIG_BYTES];   // ECDSA-P256 signature over hash
    uint32_t reserved[4];                       // Reserved for future use
} fw_image_header_t;

// ---------------------------------------------------------------------------
// DICE CDI (Compound Device Identifier)
// ---------------------------------------------------------------------------
typedef struct {
    uint8_t cdi[DICE_CDI_BYTES];                // CDI_0 value
    uint32_t flags;                             // Derivation flags
} dice_cdi_t;

// ---------------------------------------------------------------------------
// Anti-rollback SVN record
// ---------------------------------------------------------------------------
typedef struct {
    uint32_t min_svn;       // Minimum allowed SVN from OTP
    uint32_t current_svn;   // SVN from current image header
} antirollback_svn_t;

// ---------------------------------------------------------------------------
// BL0 entry point (called by Boot ROM after hardware init)
// ---------------------------------------------------------------------------
void bl0_main(void) __attribute__((noreturn));

#endif  // SAFEROOT_BL0_H_
