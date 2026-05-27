// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// test_integration_main.c -- Entry point for SafeRoot integration tests
// Runs all integration test suites and reports total pass/fail via UART.

#include "../test/test_framework.h"
#include "../hal/uart_drv.h"
#include "../hal/gpio_drv.h"
#include "../hal/hal.h"

// ---------------------------------------------------------------------------
// Global test counters (used by test_framework.h macros)
// ---------------------------------------------------------------------------
uint32_t test_pass_count = 0;
uint32_t test_fail_count = 0;

// ---------------------------------------------------------------------------
// External integration test suites
// ---------------------------------------------------------------------------
extern void test_boot_chain_suite(void);
extern void test_tpm_pcr_suite(void);
extern void test_se_crypto_suite(void);
extern void test_wdt_escalation_suite(void);
extern void test_ota_flow_suite(void);

// ---------------------------------------------------------------------------
// GPIO pin assignments for pass/fail indication
// ---------------------------------------------------------------------------
#define GPIO_PIN_PASS  0
#define GPIO_PIN_FAIL  1

// ---------------------------------------------------------------------------
// main -- integration test entry point
// ---------------------------------------------------------------------------
int main(void) {
    uart_init(115200, 100000000);
    gpio_init();
    gpio_set_output_en(GPIO_PIN_PASS, true);
    gpio_set_output_en(GPIO_PIN_FAIL, true);

    uart_puts("\n");
    uart_puts("========================================\n");
    uart_puts("  SafeRoot Integration Tests\n");
    uart_puts("========================================\n");

    // Run all integration test suites
    test_boot_chain_suite();
    test_tpm_pcr_suite();
    test_se_crypto_suite();
    test_wdt_escalation_suite();
    test_ota_flow_suite();

    // Print summary
    uart_puts("\n========================================\n");
    uart_puts("  Results: ");
    uart_put_hex(test_pass_count);
    uart_puts(" passed, ");
    uart_put_hex(test_fail_count);
    uart_puts(" failed\n");
    uart_puts("========================================\n");

    if (test_fail_count == 0) {
        uart_puts("  STATUS: ALL PASS\n");
        gpio_set_bit(GPIO_PIN_PASS);
        gpio_clear_bit(GPIO_PIN_FAIL);
    } else {
        uart_puts("  STATUS: FAIL\n");
        gpio_clear_bit(GPIO_PIN_PASS);
        gpio_set_bit(GPIO_PIN_FAIL);
    }

    while (1) {
        __asm__ volatile("wfi");
    }

    return 0;
}
