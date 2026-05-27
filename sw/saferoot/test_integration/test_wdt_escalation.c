// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// test_wdt_escalation.c -- Integration test: WDT init -> pet -> timeout
// Verifies watchdog timer behavior including window mode.
// Note: cannot test full bite escalation (reset) in a single firmware image.

#include "../test/test_framework.h"
#include "../hal/hal.h"
#include "../wdt/wdt.h"

// Short timeout for simulation -- bark at 100, bite at 200 cycles
#define TEST_BARK_TIMEOUT   100U
#define TEST_BITE_TIMEOUT   200U
#define TEST_WINDOW_MIN      50U

// ---------------------------------------------------------------------------
// Busy-wait loop to let the WDT counter advance
// ---------------------------------------------------------------------------
static void spin_cycles(uint32_t n) {
    for (volatile uint32_t i = 0; i < n; i++) {
        __asm__ volatile("nop");
    }
}

void test_wdt_escalation_suite(void) {
    TEST_SUITE_START("WDT Escalation");

    // -- Test 1: Init WDT with short timeout --
    TEST_CASE("wdt_init with bark=100, bite=200");
    wdt_init(TEST_BARK_TIMEOUT, TEST_BITE_TIMEOUT);
    uint32_t bark_reg = REG32_READ(AON_TIMER_BASE + AON_WDOG_BARK_THOLD);
    uint32_t bite_reg = REG32_READ(AON_TIMER_BASE + AON_WDOG_BITE_THOLD);
    TEST_ASSERT(bark_reg == TEST_BARK_TIMEOUT &&
                bite_reg == TEST_BITE_TIMEOUT);

    // -- Test 2: Pet resets counter --
    TEST_CASE("wdt_pet resets counter to near zero");
    wdt_enable();
    spin_cycles(10);  // let counter advance a bit
    int rc = wdt_pet();
    uint32_t cnt = wdt_get_count();
    TEST_ASSERT(rc == WDT_OK && cnt < 10);

    // -- Test 3: Counter advances after pet --
    TEST_CASE("counter advances after pet");
    wdt_pet();
    spin_cycles(30);
    cnt = wdt_get_count();
    TEST_ASSERT(cnt > 0);

    // -- Test 4: Bark interrupt fires when counter exceeds threshold --
    // Read the interrupt state register after letting counter pass bark
    TEST_CASE("bark interrupt fires after timeout");
    wdt_pet();
    // Spin enough for counter to exceed bark threshold
    spin_cycles(TEST_BARK_TIMEOUT + 50);
    uint32_t intr_state = REG32_READ(AON_TIMER_BASE + AON_INTR_STATE);
    bool bark_fired = (intr_state & (1U << INTR_STATE_WDOG_BIT)) != 0;
    TEST_ASSERT(bark_fired);

    // Disable before testing window mode
    wdt_disable();

    // -- Test 5: Window mode -- pet too early should fail --
    TEST_CASE("window mode: early pet rejected");
    wdt_init(TEST_BARK_TIMEOUT, TEST_BITE_TIMEOUT);
    wdt_set_window(TEST_WINDOW_MIN);
    wdt_enable();
    // Pet immediately -- counter near zero, below window minimum
    rc = wdt_pet();
    TEST_ASSERT_EQ(rc, WDT_ERR_WINDOW);

    // -- Test 6: Window mode -- pet after min_cycles accepted --
    TEST_CASE("window mode: pet after min accepted");
    // Spin past the window minimum
    spin_cycles(TEST_WINDOW_MIN + 20);
    rc = wdt_pet();
    TEST_ASSERT_EQ(rc, WDT_OK);

    // -- Cleanup --
    wdt_disable();
    wdt_set_window(0);  // disable window mode

    TEST_SUITE_END();
}
