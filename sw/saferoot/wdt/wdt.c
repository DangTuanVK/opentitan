// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// Watchdog Timer driver implementation for SafeRoot chip.
// Bare-metal, rv32imc, no OS.

#include "wdt.h"
#include "../hal/hal.h"   // REG32() macro

// ----------------------------------------------------------------
// Internal state
// ----------------------------------------------------------------

// Window-watchdog minimum count.  When non-zero, wdt_pet() will reject
// a pet if the counter has not yet reached this value.
static uint32_t wdt_window_min = 0;

// Configured thresholds (kept so wdt_enable can re-apply them).
static uint32_t cfg_bark_timeout = 0;
static uint32_t cfg_bite_timeout = 0;

// Track whether the driver has been initialised.
static bool wdt_initialised = false;

// ----------------------------------------------------------------
// Helper: read / write AON timer registers
// ----------------------------------------------------------------

static inline uint32_t aon_read(uint32_t offset) {
    return REG32(AON_TIMER_BASE + offset);
}

static inline void aon_write(uint32_t offset, uint32_t value) {
    REG32(AON_TIMER_BASE + offset) = value;
}

// ----------------------------------------------------------------
// Fence helpers -- AON domain is asynchronous to the main clock.
// A write-then-read round-trip ensures the value has landed.
// ----------------------------------------------------------------

static inline void aon_fence(uint32_t offset) {
    (void)aon_read(offset);
}

// ----------------------------------------------------------------
// wdt_init
// ----------------------------------------------------------------
void wdt_init(uint32_t bark_timeout, uint32_t bite_timeout) {
    // Disable the watchdog while we reconfigure.
    uint32_t ctrl = aon_read(AON_WDOG_CTRL);
    ctrl &= ~(1u << WDOG_CTRL_ENABLE_BIT);
    aon_write(AON_WDOG_CTRL, ctrl);
    aon_fence(AON_WDOG_CTRL);

    // Clear any pending watchdog interrupt.
    aon_write(AON_INTR_STATE, (1u << INTR_STATE_WDOG_BIT));
    aon_fence(AON_INTR_STATE);

    // Reset the counter to zero.
    aon_write(AON_WDOG_COUNT, 0u);
    aon_fence(AON_WDOG_COUNT);

    // Program bark and bite thresholds.
    aon_write(AON_WDOG_BARK_THOLD, bark_timeout);
    aon_fence(AON_WDOG_BARK_THOLD);

    aon_write(AON_WDOG_BITE_THOLD, bite_timeout);
    aon_fence(AON_WDOG_BITE_THOLD);

    // Save configuration.
    cfg_bark_timeout = bark_timeout;
    cfg_bite_timeout = bite_timeout;
    wdt_window_min   = 0;  // window mode off by default
    wdt_initialised  = true;
}

// ----------------------------------------------------------------
// wdt_enable
// ----------------------------------------------------------------
void wdt_enable(void) {
    if (!wdt_initialised) {
        return;
    }

    // Clear any stale interrupt before enabling.
    aon_write(AON_INTR_STATE, (1u << INTR_STATE_WDOG_BIT));

    // Reset the counter so the full timeout window is available.
    aon_write(AON_WDOG_COUNT, 0u);
    aon_fence(AON_WDOG_COUNT);

    // Enable the watchdog.  Keep pause_in_sleep enabled so the
    // watchdog does not fire during legitimate low-power states.
    uint32_t ctrl = (1u << WDOG_CTRL_ENABLE_BIT) |
                    (1u << WDOG_CTRL_PAUSE_IN_SLEEP_BIT);
    aon_write(AON_WDOG_CTRL, ctrl);
    aon_fence(AON_WDOG_CTRL);
}

// ----------------------------------------------------------------
// wdt_disable
// ----------------------------------------------------------------
void wdt_disable(void) {
    uint32_t ctrl = aon_read(AON_WDOG_CTRL);
    ctrl &= ~(1u << WDOG_CTRL_ENABLE_BIT);
    aon_write(AON_WDOG_CTRL, ctrl);
    aon_fence(AON_WDOG_CTRL);
}

// ----------------------------------------------------------------
// wdt_pet
// ----------------------------------------------------------------
int wdt_pet(void) {
    // Check the watchdog is actually running.
    uint32_t ctrl = aon_read(AON_WDOG_CTRL);
    if (!(ctrl & (1u << WDOG_CTRL_ENABLE_BIT))) {
        return WDT_ERR_DISABLED;
    }

    // Window-watchdog check: if a minimum window is configured, reject
    // pets that arrive before the counter reaches the threshold.
    if (wdt_window_min != 0) {
        uint32_t current = aon_read(AON_WDOG_COUNT);
        if (current < wdt_window_min) {
            // Too early -- this may indicate a fault or an attacker
            // trying to suppress the watchdog by petting too fast.
            return WDT_ERR_WINDOW;
        }
    }

    // Clear any pending bark interrupt.
    aon_write(AON_INTR_STATE, (1u << INTR_STATE_WDOG_BIT));

    // Reset the counter (pet the dog).
    aon_write(AON_WDOG_COUNT, 0u);
    aon_fence(AON_WDOG_COUNT);

    return WDT_OK;
}

// ----------------------------------------------------------------
// wdt_set_window
// ----------------------------------------------------------------
void wdt_set_window(uint32_t min_cycles) {
    // Sanity: the window minimum must be less than the bark threshold,
    // otherwise petting is impossible.
    if (min_cycles >= cfg_bark_timeout && cfg_bark_timeout != 0) {
        // Clamp to half the bark timeout so the system stays alive.
        min_cycles = cfg_bark_timeout / 2u;
    }
    wdt_window_min = min_cycles;
}

// ----------------------------------------------------------------
// wdt_get_count
// ----------------------------------------------------------------
uint32_t wdt_get_count(void) {
    return aon_read(AON_WDOG_COUNT);
}

// ----------------------------------------------------------------
// Watchdog bark ISR stub.
// Link this to the AON timer interrupt in the PLIC / vectored CLIC.
// It clears the interrupt and asserts the ALERT_N pin so that the
// external supervisor is notified that a reset is imminent.
// ----------------------------------------------------------------

// GPIO base and pin definitions (SafeRoot memory map)
#define GPIO_BASE       0x40210000u
#define GPIO_DIRECT_OUT 0x04u          // Direct output register

#define MIO_ALERT_N_PIN  4u
#define MIO_FAILSAFE_PIN 6u

void wdt_bark_isr(void) {
    // 1. Clear the bark interrupt so it does not re-fire immediately.
    aon_write(AON_INTR_STATE, (1u << INTR_STATE_WDOG_BIT));

    // 2. Assert ALERT_N (active-low) -- pull MIO[4] low.
    uint32_t gpio_out = REG32(GPIO_BASE + GPIO_DIRECT_OUT);
    gpio_out &= ~(1u << MIO_ALERT_N_PIN);
    REG32(GPIO_BASE + GPIO_DIRECT_OUT) = gpio_out;

    // 3. Attempt one last pet.  If the main firmware is stuck, this
    //    will fail with WDT_ERR_WINDOW or the bite timer will fire
    //    before the next bark, causing a hard reset.
    //    (Intentionally *not* calling wdt_pet() here to let the bite
    //    timer proceed -- the ISR is only for notification.)
}
