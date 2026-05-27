// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// otp_drv.h — OTP Controller driver for SafeRoot (OpenTitan OTP_CTRL IP)
// Provides read-only access to OTP partitions via the DAI interface.

#ifndef SAFEROOT_OTP_DRV_H_
#define SAFEROOT_OTP_DRV_H_

#include "hal.h"

// OTP partition IDs (OpenTitan standard partitions)
typedef enum {
    kOtpPartCreatorSwCfg   = 0,
    kOtpPartOwnerSwCfg     = 1,
    kOtpPartHwCfg0         = 2,
    kOtpPartHwCfg1         = 3,
    kOtpPartSecret0        = 4,
    kOtpPartSecret1        = 5,
    kOtpPartSecret2        = 6,
    kOtpPartLifeCycle       = 7,
} otp_partition_t;

// Initialize OTP controller (check status, clear errors)
hal_status_t otp_init(void);

// Read a single 32-bit word from OTP via Direct Access Interface (DAI).
// addr: byte address within OTP (must be 4-byte aligned).
// value: pointer to store the read value.
hal_status_t otp_read_word(uint32_t addr, uint32_t *value);

// Read a 64-bit value from OTP via DAI.
// addr: byte address within OTP (must be 8-byte aligned).
hal_status_t otp_read_word64(uint32_t addr, uint32_t *value_lo,
                             uint32_t *value_hi);

// Check if OTP controller is idle
bool otp_is_idle(void);

// Write a 32-bit word to OTP (IRREVERSIBLE — one-time programmable)
hal_status_t otp_write_word(uint32_t addr, uint32_t value);

#endif  // SAFEROOT_OTP_DRV_H_
