// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// test_main.c — Entry point for SafeRoot HAL unit test suite
// Runs on bare-metal (Verilator / FPGA), outputs results via UART.

#include "test_framework.h"
#include "../hal/uart_drv.h"
#include "../hal/gpio_drv.h"
#include "../hal/hal.h"

// ---------------------------------------------------------------------------
// Global test counters (used by test_framework.h macros)
// ---------------------------------------------------------------------------
uint32_t test_pass_count = 0;
uint32_t test_fail_count = 0;

// ---------------------------------------------------------------------------
// External test suite functions
// ---------------------------------------------------------------------------
extern void test_uart_suite(void);
extern void test_gpio_suite(void);
extern void test_crypto_suite(void);
extern void test_wdt_suite(void);

// ---------------------------------------------------------------------------
// GPIO pin assignments for pass/fail indication
// ---------------------------------------------------------------------------
#define GPIO_PIN_PASS  0   // GPIO[0] = high on all-pass
#define GPIO_PIN_FAIL  1   // GPIO[1] = high on any failure

// ---------------------------------------------------------------------------
// main — test entry point
// ---------------------------------------------------------------------------
int main(void) {
    // Initialise UART for test output (115200 baud, 100 MHz clock)
    uart_init(115200, 100000000);

    // Initialise GPIO for pass/fail signalling
    gpio_init();
    gpio_set_output_en(GPIO_PIN_PASS, true);
    gpio_set_output_en(GPIO_PIN_FAIL, true);

    uart_puts("\n");
    uart_puts("========================================\n");
    uart_puts("  SafeRoot HAL Unit Tests\n");
    uart_puts("========================================\n");

    // Run all test suites
    test_uart_suite();
    test_gpio_suite();
    test_crypto_suite();
    test_wdt_suite();

    // Print summary
    uart_puts("\n========================================\n");
    uart_puts("  Results: ");
    uart_put_hex(test_pass_count);
    uart_puts(" passed, ");
    uart_put_hex(test_fail_count);
    uart_puts(" failed\n");
    uart_puts("========================================\n");

    // Signal pass/fail via GPIO
    if (test_fail_count == 0) {
        uart_puts("  STATUS: ALL PASS\n");
        gpio_set_bit(GPIO_PIN_PASS);
        gpio_clear_bit(GPIO_PIN_FAIL);
    } else {
        uart_puts("  STATUS: FAIL\n");
        gpio_clear_bit(GPIO_PIN_PASS);
        gpio_set_bit(GPIO_PIN_FAIL);
    }

    // Halt — in simulation, Verilator monitors GPIO[0]/GPIO[1] to
    // determine test result and terminate.
    while (1) {
        __asm__ volatile("wfi");
    }

    return 0;
}
