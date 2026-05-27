// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// Watchdog Timer (WDT) driver for SafeRoot chip.
// Uses the AON Timer IP at 0x40320000.

#ifndef SAFEROOT_WDT_H_
#define SAFEROOT_WDT_H_

#include <stdint.h>
#include <stdbool.h>

// ----------------------------------------------------------------
// AON Timer base address (SafeRoot memory map)
// ----------------------------------------------------------------
#define AON_TIMER_BASE  0x40320000u

// ----------------------------------------------------------------
// AON Timer watchdog registers (offsets from base)
// ----------------------------------------------------------------
#define AON_WKUP_CTRL       0x00u  // Wakeup control (enable + prescaler)
#define AON_WKUP_THOLD      0x04u  // Wakeup threshold
#define AON_WKUP_COUNT      0x08u  // Wakeup counter
#define AON_WDOG_CTRL       0x0Cu  // Watchdog control (enable + pause_in_sleep)
#define AON_WDOG_BARK_THOLD 0x10u  // Bark threshold (fires interrupt)
#define AON_WDOG_BITE_THOLD 0x14u  // Bite threshold (fires reset)
#define AON_WDOG_COUNT      0x18u  // Watchdog counter value
#define AON_INTR_STATE      0x1Cu  // Interrupt status

// ----------------------------------------------------------------
// WDOG_CTRL bit positions
// ----------------------------------------------------------------
#define WDOG_CTRL_ENABLE_BIT        0u
#define WDOG_CTRL_PAUSE_IN_SLEEP_BIT 1u

// ----------------------------------------------------------------
// INTR_STATE bit positions
// ----------------------------------------------------------------
#define INTR_STATE_WKUP_BIT  0u
#define INTR_STATE_WDOG_BIT  1u

// ----------------------------------------------------------------
// Return codes
// ----------------------------------------------------------------
#define WDT_OK              0
#define WDT_ERR_WINDOW     -1  // Pet arrived too early (window violation)
#define WDT_ERR_DISABLED   -2  // Watchdog is not enabled

// ----------------------------------------------------------------
// Public API
// ----------------------------------------------------------------

/**
 * Initialise the watchdog with bark and bite thresholds.
 *
 * @param bark_timeout  Cycle count at which the watchdog fires an interrupt.
 * @param bite_timeout  Cycle count at which the watchdog fires a reset.
 *                      Must be > bark_timeout.
 */
void wdt_init(uint32_t bark_timeout, uint32_t bite_timeout);

/**
 * Pet (kick) the watchdog -- resets the counter to zero.
 *
 * @return WDT_OK on success, WDT_ERR_WINDOW if the pet arrived before the
 *         minimum window, WDT_ERR_DISABLED if watchdog is off.
 */
int wdt_pet(void);

/** Enable the watchdog counter. */
void wdt_enable(void);

/** Disable the watchdog counter. */
void wdt_disable(void);

/**
 * Configure a window watchdog.  Pets that arrive before min_cycles have
 * elapsed since the last pet are rejected with WDT_ERR_WINDOW.
 *
 * @param min_cycles  Minimum counter value before a pet is accepted.
 *                    Pass 0 to disable window mode.
 */
void wdt_set_window(uint32_t min_cycles);

/** Read the current watchdog counter value. */
uint32_t wdt_get_count(void);

#endif  // SAFEROOT_WDT_H_
