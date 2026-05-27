// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// hal.h — SafeRoot HAL common header
// Bare-metal register access macros, base addresses, and return types.

#ifndef SAFEROOT_HAL_H_
#define SAFEROOT_HAL_H_

#include <stdint.h>
#include <stdbool.h>

// ---------------------------------------------------------------------------
// Register access macros (volatile 32-bit read/write)
// ---------------------------------------------------------------------------
#define REG32(addr)       (*(volatile uint32_t *)(addr))
#define REG32_READ(addr)  (*(volatile uint32_t *)(addr))
#define REG32_WRITE(addr, val) \
    do { (*(volatile uint32_t *)(addr)) = (val); } while (0)

// Bit manipulation helpers
#define BIT(n)            (1U << (n))
#define FIELD_GET(reg, mask, shift) (((reg) >> (shift)) & (mask))

// ---------------------------------------------------------------------------
// SafeRoot peripheral base addresses (from top_saferoot_pkg.sv)
// ---------------------------------------------------------------------------
#define SAFEROOT_UART0_BASE       0x40230000U
#define SAFEROOT_GPIO_BASE        0x40210000U
#define SAFEROOT_AES_BASE         0x40000000U
#define SAFEROOT_HMAC_BASE        0x40010000U
#define SAFEROOT_SPI_DEVICE_BASE  0x40200000U
#define SAFEROOT_OTP_CTRL_BASE    0x40130000U
#define SAFEROOT_KEYMGR_BASE      0x40100000U
#define SAFEROOT_FLASH_CTRL_BASE  0x40170000U  // Register interface
#define SAFEROOT_FLASH_MEM_BASE   0x20000000U  // Memory-mapped flash
#define SAFEROOT_AON_TIMER_BASE   0x40320000U

// ---------------------------------------------------------------------------
// Common return type for HAL functions
// ---------------------------------------------------------------------------
typedef enum {
    kHalOk          = 0,  // Success
    kHalError       = 1,  // Generic error
    kHalBusy        = 2,  // Peripheral busy
    kHalTimeout     = 3,  // Operation timed out
    kHalBadArg      = 4,  // Invalid argument
} hal_status_t;

// Default polling timeout (loop iterations)
#define HAL_TIMEOUT_CYCLES  100000U

// Memory barrier for RISC-V (fence instruction)
#define HAL_BARRIER() __asm__ volatile("fence" ::: "memory")

#endif  // SAFEROOT_HAL_H_
