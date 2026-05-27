// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// test_framework.h — Minimal bare-metal test framework for SafeRoot
// Output goes through UART (no printf, no malloc).

#ifndef SAFEROOT_TEST_FRAMEWORK_H_
#define SAFEROOT_TEST_FRAMEWORK_H_

#include "../hal/uart_drv.h"

// ---------------------------------------------------------------------------
// Global counters (defined in test_main.c)
// ---------------------------------------------------------------------------
extern uint32_t test_pass_count;
extern uint32_t test_fail_count;

// ---------------------------------------------------------------------------
// Test suite / case macros
// ---------------------------------------------------------------------------
#define TEST_SUITE_START(name) \
    do { uart_puts("\n=== Suite: " name " ===\n"); } while (0)

#define TEST_SUITE_END() \
    do { uart_puts("--- suite done ---\n"); } while (0)

#define TEST_CASE(name) \
    uart_puts("  [TEST] " name ": ")

// ---------------------------------------------------------------------------
// Assertion macros
// ---------------------------------------------------------------------------
#define TEST_ASSERT(cond)                       \
    do {                                        \
        if (cond) {                             \
            uart_puts("PASS\n");                \
            test_pass_count++;                  \
        } else {                                \
            uart_puts("FAIL\n");                \
            test_fail_count++;                  \
        }                                       \
    } while (0)

#define TEST_ASSERT_EQ(a, b)  TEST_ASSERT((a) == (b))

#define TEST_ASSERT_MEM_EQ(a, b, len)                           \
    do {                                                        \
        const uint8_t *_pa = (const uint8_t *)(a);              \
        const uint8_t *_pb = (const uint8_t *)(b);              \
        bool _match = true;                                     \
        for (uint32_t _i = 0; _i < (len); _i++) {              \
            if (_pa[_i] != _pb[_i]) { _match = false; break; } \
        }                                                       \
        TEST_ASSERT(_match);                                    \
    } while (0)

#endif  // SAFEROOT_TEST_FRAMEWORK_H_
