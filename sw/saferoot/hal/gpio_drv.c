// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// gpio_drv.c — GPIO driver for SafeRoot (OpenTitan GPIO IP)
//
// OpenTitan GPIO has 32 pins with masked writes to avoid read-modify-write
// hazards. The MASKED_OUT_LOWER/UPPER registers use the upper 16 bits as
// a write mask and the lower 16 bits as the value.

#include "gpio_drv.h"

// ---------------------------------------------------------------------------
// GPIO register offsets (OpenTitan GPIO IP, standard layout)
// ---------------------------------------------------------------------------
#define GPIO_INTR_STATE         0x00
#define GPIO_INTR_ENABLE        0x04
#define GPIO_INTR_TEST          0x08
#define GPIO_ALERT_TEST         0x0C
#define GPIO_DATA_IN            0x10  // RO — current pin values
#define GPIO_DIRECT_OUT         0x14  // RW — direct output value
#define GPIO_MASKED_OUT_LOWER   0x18  // RW — masked write pins [15:0]
#define GPIO_MASKED_OUT_UPPER   0x1C  // RW — masked write pins [31:16]
#define GPIO_DIRECT_OE          0x20  // RW — output enable
#define GPIO_MASKED_OE_LOWER    0x24  // RW — masked OE pins [15:0]
#define GPIO_MASKED_OE_UPPER    0x28  // RW — masked OE pins [31:16]
#define GPIO_INTR_CTRL_EN_RISING   0x2C
#define GPIO_INTR_CTRL_EN_FALLING  0x30
#define GPIO_INTR_CTRL_EN_LVLHIGH  0x34
#define GPIO_INTR_CTRL_EN_LVLLOW   0x38

#define GPIO_BASE SAFEROOT_GPIO_BASE

// ---------------------------------------------------------------------------
// Masked write helper
// For pins [15:0]:  write to MASKED_OUT_LOWER, mask in [31:16], value in [15:0]
// For pins [31:16]: write to MASKED_OUT_UPPER, mask in [31:16], value in [15:0]
//                   (pin number is shifted down by 16)
// ---------------------------------------------------------------------------
static void gpio_masked_write(uint32_t pin, bool value) {
    if (pin < 16) {
        uint32_t mask = 1U << (pin + 16);  // mask bit in upper halfword
        uint32_t val  = value ? (1U << pin) : 0;
        REG32_WRITE(GPIO_BASE + GPIO_MASKED_OUT_LOWER, mask | val);
    } else {
        uint32_t p = pin - 16;
        uint32_t mask = 1U << (p + 16);
        uint32_t val  = value ? (1U << p) : 0;
        REG32_WRITE(GPIO_BASE + GPIO_MASKED_OUT_UPPER, mask | val);
    }
}

static void gpio_masked_oe(uint32_t pin, bool enable) {
    if (pin < 16) {
        uint32_t mask = 1U << (pin + 16);
        uint32_t val  = enable ? (1U << pin) : 0;
        REG32_WRITE(GPIO_BASE + GPIO_MASKED_OE_LOWER, mask | val);
    } else {
        uint32_t p = pin - 16;
        uint32_t mask = 1U << (p + 16);
        uint32_t val  = enable ? (1U << p) : 0;
        REG32_WRITE(GPIO_BASE + GPIO_MASKED_OE_UPPER, mask | val);
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

hal_status_t gpio_init(void) {
    // Disable all outputs
    REG32_WRITE(GPIO_BASE + GPIO_DIRECT_OE, 0);
    // Clear all output values
    REG32_WRITE(GPIO_BASE + GPIO_DIRECT_OUT, 0);
    // Clear pending interrupts
    REG32_WRITE(GPIO_BASE + GPIO_INTR_STATE, 0xFFFFFFFF);
    // Disable all interrupt sources
    REG32_WRITE(GPIO_BASE + GPIO_INTR_ENABLE, 0);
    REG32_WRITE(GPIO_BASE + GPIO_INTR_CTRL_EN_RISING, 0);
    REG32_WRITE(GPIO_BASE + GPIO_INTR_CTRL_EN_FALLING, 0);
    REG32_WRITE(GPIO_BASE + GPIO_INTR_CTRL_EN_LVLHIGH, 0);
    REG32_WRITE(GPIO_BASE + GPIO_INTR_CTRL_EN_LVLLOW, 0);
    return kHalOk;
}

uint32_t gpio_read(void) {
    return REG32_READ(GPIO_BASE + GPIO_DATA_IN);
}

uint32_t gpio_read_bit(uint32_t pin) {
    if (pin > 31) return 0;
    return (REG32_READ(GPIO_BASE + GPIO_DATA_IN) >> pin) & 1U;
}

void gpio_write(uint32_t val) {
    REG32_WRITE(GPIO_BASE + GPIO_DIRECT_OUT, val);
}

void gpio_set_bit(uint32_t pin) {
    if (pin > 31) return;
    gpio_masked_write(pin, true);
}

void gpio_clear_bit(uint32_t pin) {
    if (pin > 31) return;
    gpio_masked_write(pin, false);
}

void gpio_toggle(uint32_t pin) {
    if (pin > 31) return;
    uint32_t current = REG32_READ(GPIO_BASE + GPIO_DIRECT_OUT);
    current ^= (1U << pin);
    REG32_WRITE(GPIO_BASE + GPIO_DIRECT_OUT, current);
}

void gpio_set_output_en(uint32_t pin, bool enable) {
    if (pin > 31) return;
    gpio_masked_oe(pin, enable);
}
