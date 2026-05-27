// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// otp_drv.c — OTP Controller driver for SafeRoot (OpenTitan OTP_CTRL IP)
//
// Uses the Direct Access Interface (DAI) for reading OTP words.
// Register offsets based on OpenTitan otp_ctrl register layout.
//
// DAI protocol: write address to DAI_ADDRESS -> trigger DAI read command
//               via DAI_CMD -> poll STATUS.DAI_IDLE -> read DAI_RDATA

#include "otp_drv.h"

// ---------------------------------------------------------------------------
// OTP_CTRL register offsets (OpenTitan standard layout)
// ---------------------------------------------------------------------------
#define OTP_INTR_STATE         0x00
#define OTP_INTR_ENABLE        0x04
#define OTP_INTR_TEST          0x08
#define OTP_ALERT_TEST         0x0C
#define OTP_STATUS             0x10  // Status register (RO)
#define OTP_ERR_CODE_0         0x14  // Error code registers
// ... error codes for each partition
#define OTP_DIRECT_ACCESS_REGWEN 0x38
#define OTP_DIRECT_ACCESS_CMD  0x3C  // DAI command trigger
#define OTP_DIRECT_ACCESS_ADDR 0x40  // DAI address
#define OTP_DIRECT_ACCESS_WDATA_0 0x44  // DAI write data [31:0]
#define OTP_DIRECT_ACCESS_WDATA_1 0x48  // DAI write data [63:32]
#define OTP_DIRECT_ACCESS_RDATA_0 0x4C  // DAI read data [31:0]
#define OTP_DIRECT_ACCESS_RDATA_1 0x50  // DAI read data [63:32]
#define OTP_CHECK_TRIGGER      0x54
#define OTP_CHECK_REGWEN       0x58
#define OTP_CHECK_TIMEOUT      0x5C
#define OTP_INTEGRITY_CHECK_PERIOD 0x60
#define OTP_CONSISTENCY_CHECK_PERIOD 0x64

// ---------------------------------------------------------------------------
// STATUS register bit definitions
// ---------------------------------------------------------------------------
// Each partition has an error/idle bit. The DAI bits are:
// Bit layout varies by implementation. Key bits:
#define OTP_STATUS_DAI_IDLE    BIT(16)  // DAI is idle
#define OTP_STATUS_DAI_ERROR   BIT(17)  // DAI error occurred
#define OTP_STATUS_CHECK_PENDING BIT(18)

// ---------------------------------------------------------------------------
// DIRECT_ACCESS_CMD values
// ---------------------------------------------------------------------------
// [0] DAI read command
// [1] DAI write command
// [2] DAI digest command
#define OTP_DAI_CMD_READ       0x01
#define OTP_DAI_CMD_WRITE      0x02
#define OTP_DAI_CMD_DIGEST     0x04

// ---------------------------------------------------------------------------
// DIRECT_ACCESS_REGWEN — lock bit
// ---------------------------------------------------------------------------
#define OTP_DAI_REGWEN_BIT     BIT(0)

#define OTP_BASE SAFEROOT_OTP_CTRL_BASE

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static hal_status_t otp_wait_dai_idle(void) {
    for (uint32_t i = 0; i < HAL_TIMEOUT_CYCLES; i++) {
        uint32_t st = REG32_READ(OTP_BASE + OTP_STATUS);
        if (st & OTP_STATUS_DAI_ERROR) {
            return kHalError;
        }
        if (st & OTP_STATUS_DAI_IDLE) {
            return kHalOk;
        }
    }
    return kHalTimeout;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool otp_is_idle(void) {
    return (REG32_READ(OTP_BASE + OTP_STATUS) & OTP_STATUS_DAI_IDLE) != 0;
}

hal_status_t otp_init(void) {
    // Clear pending interrupts
    REG32_WRITE(OTP_BASE + OTP_INTR_STATE, 0xFFFFFFFF);
    // Disable interrupts (polling mode)
    REG32_WRITE(OTP_BASE + OTP_INTR_ENABLE, 0);
    // Wait for DAI to be idle after reset
    return otp_wait_dai_idle();
}

hal_status_t otp_read_word(uint32_t addr, uint32_t *value) {
    if (value == (void *)0) return kHalBadArg;
    if (addr & 0x3) return kHalBadArg;  // Must be 4-byte aligned

    // Check DAI write access is enabled
    if (!(REG32_READ(OTP_BASE + OTP_DIRECT_ACCESS_REGWEN) & OTP_DAI_REGWEN_BIT)) {
        return kHalBusy;
    }

    // Wait for idle
    hal_status_t st = otp_wait_dai_idle();
    if (st != kHalOk) return st;

    // Set address
    REG32_WRITE(OTP_BASE + OTP_DIRECT_ACCESS_ADDR, addr);

    // Trigger DAI read
    REG32_WRITE(OTP_BASE + OTP_DIRECT_ACCESS_CMD, OTP_DAI_CMD_READ);

    // Wait for completion
    st = otp_wait_dai_idle();
    if (st != kHalOk) return st;

    // Read result
    *value = REG32_READ(OTP_BASE + OTP_DIRECT_ACCESS_RDATA_0);

    return kHalOk;
}

hal_status_t otp_read_word64(uint32_t addr, uint32_t *value_lo,
                             uint32_t *value_hi) {
    if (value_lo == (void *)0 || value_hi == (void *)0) return kHalBadArg;
    if (addr & 0x7) return kHalBadArg;  // Must be 8-byte aligned

    // Check DAI access is enabled
    if (!(REG32_READ(OTP_BASE + OTP_DIRECT_ACCESS_REGWEN) & OTP_DAI_REGWEN_BIT)) {
        return kHalBusy;
    }

    // Wait for idle
    hal_status_t st = otp_wait_dai_idle();
    if (st != kHalOk) return st;

    // Set address
    REG32_WRITE(OTP_BASE + OTP_DIRECT_ACCESS_ADDR, addr);

    // Trigger DAI read
    REG32_WRITE(OTP_BASE + OTP_DIRECT_ACCESS_CMD, OTP_DAI_CMD_READ);

    // Wait for completion
    st = otp_wait_dai_idle();
    if (st != kHalOk) return st;

    // Read 64-bit result
    *value_lo = REG32_READ(OTP_BASE + OTP_DIRECT_ACCESS_RDATA_0);
    *value_hi = REG32_READ(OTP_BASE + OTP_DIRECT_ACCESS_RDATA_1);

    return kHalOk;
}

// Write a 32-bit word to OTP via DAI (one-time programmable — irreversible!)
hal_status_t otp_write_word(uint32_t addr, uint32_t value) {
    REG32_WRITE(OTP_BASE + OTP_DIRECT_ACCESS_ADDR, addr);
    REG32_WRITE(OTP_BASE + OTP_DIRECT_ACCESS_WDATA_0, value);
    REG32_WRITE(OTP_BASE + OTP_DIRECT_ACCESS_CMD, 0x2);  // write cmd
    return otp_wait_dai_idle();
}
