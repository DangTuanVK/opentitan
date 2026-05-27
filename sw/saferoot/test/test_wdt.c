// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// test_wdt.c — Unit tests for Watchdog Timer driver

#include "test_framework.h"
#include "../wdt/wdt.h"
#include "../hal/hal.h"

// AON timer register base and offsets (mirror from wdt.h)
#define AON_BASE        AON_TIMER_BASE
#define AON_WDOG_CTRL_OFF   AON_WDOG_CTRL
#define AON_BARK_OFF        AON_WDOG_BARK_THOLD
#define AON_BITE_OFF        AON_WDOG_BITE_THOLD
#define AON_COUNT_OFF       AON_WDOG_COUNT

void test_wdt_suite(void) {
    TEST_SUITE_START("WDT");

    // ---- Test wdt_init: verify AON timer registers ----
    TEST_CASE("wdt_init sets bark threshold");
    wdt_init(1000, 2000);
    uint32_t bark = REG32_READ(AON_BASE + AON_BARK_OFF);
    TEST_ASSERT_EQ(bark, 1000);

    TEST_CASE("wdt_init sets bite threshold");
    uint32_t bite = REG32_READ(AON_BASE + AON_BITE_OFF);
    TEST_ASSERT_EQ(bite, 2000);

    TEST_CASE("wdt_init resets counter to 0");
    uint32_t cnt = REG32_READ(AON_BASE + AON_COUNT_OFF);
    TEST_ASSERT_EQ(cnt, 0);

    // ---- Test wdt_enable / wdt_pet ----
    TEST_CASE("wdt_pet resets counter");
    wdt_enable();
    // Counter may have advanced slightly; pet should reset it.
    int rc = wdt_pet();
    cnt = wdt_get_count();
    // Counter should be very close to 0 after pet
    TEST_ASSERT(rc == WDT_OK && cnt < 10);

    // ---- Test wdt_pet when disabled returns error ----
    TEST_CASE("wdt_pet when disabled returns ERR_DISABLED");
    wdt_disable();
    rc = wdt_pet();
    TEST_ASSERT_EQ(rc, WDT_ERR_DISABLED);

    // ---- Test window watchdog: early pet should return WDT_ERR_WINDOW ----
    TEST_CASE("window WDT rejects early pet");
    wdt_init(10000, 20000);
    wdt_set_window(5000);   // min 5000 cycles before pet accepted
    wdt_enable();
    // Pet immediately — counter is near zero, should be rejected
    rc = wdt_pet();
    TEST_ASSERT_EQ(rc, WDT_ERR_WINDOW);

    // Clean up: disable watchdog so it does not bite during other tests
    wdt_disable();

    TEST_SUITE_END();
}
