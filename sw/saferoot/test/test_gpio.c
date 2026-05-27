// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// test_gpio.c — Unit tests for GPIO HAL driver

#include "test_framework.h"
#include "../hal/gpio_drv.h"
#include "../hal/hal.h"

// GPIO register offsets (mirror from gpio_drv.c for verification)
#define GPIO_BASE       SAFEROOT_GPIO_BASE
#define GPIO_DIRECT_OUT 0x14
#define GPIO_DIRECT_OE  0x20

void test_gpio_suite(void) {
    TEST_SUITE_START("GPIO");

    // ---- Test gpio_init: verify OE register is cleared ----
    TEST_CASE("gpio_init clears OE");
    hal_status_t st = gpio_init();
    uint32_t oe = REG32_READ(GPIO_BASE + GPIO_DIRECT_OE);
    TEST_ASSERT(st == kHalOk && oe == 0);

    // ---- Test gpio_init: verify DIRECT_OUT is cleared ----
    TEST_CASE("gpio_init clears DIRECT_OUT");
    uint32_t out = REG32_READ(GPIO_BASE + GPIO_DIRECT_OUT);
    TEST_ASSERT_EQ(out, 0);

    // ---- Test gpio_set_bit: verify DIRECT_OUT ----
    TEST_CASE("gpio_set_bit pin 3");
    gpio_set_output_en(3, true);
    gpio_set_bit(3);
    out = REG32_READ(GPIO_BASE + GPIO_DIRECT_OUT);
    TEST_ASSERT((out & BIT(3)) != 0);

    // ---- Test gpio_clear_bit: verify DIRECT_OUT ----
    TEST_CASE("gpio_clear_bit pin 3");
    gpio_clear_bit(3);
    out = REG32_READ(GPIO_BASE + GPIO_DIRECT_OUT);
    TEST_ASSERT((out & BIT(3)) == 0);

    // ---- Test gpio_toggle: toggle pin 5 twice ----
    TEST_CASE("gpio_toggle pin 5");
    gpio_set_output_en(5, true);
    gpio_clear_bit(5);
    gpio_toggle(5);
    out = REG32_READ(GPIO_BASE + GPIO_DIRECT_OUT);
    bool first = (out & BIT(5)) != 0;
    gpio_toggle(5);
    out = REG32_READ(GPIO_BASE + GPIO_DIRECT_OUT);
    bool second = (out & BIT(5)) == 0;
    TEST_ASSERT(first && second);

    // ---- Test gpio_write: set all bits, then clear ----
    TEST_CASE("gpio_write 0xA5A5A5A5");
    gpio_write(0xA5A5A5A5);
    out = REG32_READ(GPIO_BASE + GPIO_DIRECT_OUT);
    TEST_ASSERT_EQ(out, 0xA5A5A5A5);

    // Clean up
    gpio_init();

    TEST_SUITE_END();
}
