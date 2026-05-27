// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// Alert Handler escalation policy implementation for SafeRoot chip.
// Bare-metal, rv32imc, no OS.
//
// Escalation ladder:
//   Class A (L1) -- NMI after 5 s
//   Class B (L2) -- Chip reset after 10 s
//   Class C (L3) -- Zeroize all secrets after 15 s
//   Class D (L4) -- Move lifecycle to SCRAP (permanent brick)

#include "alert_config.h"
#include "../hal/hal.h"  // REG32() macro

// ----------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------

static inline uint32_t alert_read(uint32_t offset) {
    return REG32(ALERT_HANDLER_BASE + offset);
}

static inline void alert_write(uint32_t offset, uint32_t value) {
    REG32(ALERT_HANDLER_BASE + offset) = value;
}

/**
 * Return the register base for a given alert class (0..3).
 */
static inline uint32_t class_base(uint32_t cls) {
    return ALERT_CLASS_BASE + cls * ALERT_CLASS_STRIDE;
}

/**
 * Configure one escalation class.
 *
 * @param cls           Class index (0 = A, 1 = B, 2 = C, 3 = D).
 * @param accum_thresh  Number of alerts before escalation starts.
 * @param timeout_cyc   IRQ timeout in AON cycles (0 = escalate immediately
 *                      after accumulator threshold).
 * @param phase0_cyc    Phase 0 duration in AON cycles.
 */
static void configure_class(uint32_t cls, uint32_t accum_thresh,
                            uint32_t timeout_cyc, uint32_t phase0_cyc) {
    uint32_t base = class_base(cls);

    // Clear any prior accumulator state.
    alert_write(base + CLASS_CLR, 1u);

    // Set accumulator threshold.
    alert_write(base + CLASS_ACCUM_THRESH, accum_thresh);

    // Set IRQ timeout (cycles before escalation kicks in after IRQ).
    alert_write(base + CLASS_TIMEOUT, timeout_cyc);

    // Set phase 0 duration.  Phases 1 and 2 are set to zero so that
    // escalation proceeds to the terminal action without extra delay.
    alert_write(base + CLASS_PHASE0_CYC, phase0_cyc);
    alert_write(base + CLASS_PHASE1_CYC, 0u);
    alert_write(base + CLASS_PHASE2_CYC, 0u);

    // Enable the class with escalation, then lock the configuration
    // so it cannot be altered by software after boot.
    uint32_t ctrl = (1u << CLASS_CTRL_EN_BIT) |
                    (1u << CLASS_CTRL_LOCK_BIT) |
                    (1u << CLASS_CTRL_ESC_EN_BIT);
    alert_write(base + CLASS_CTRL, ctrl);
}

// ----------------------------------------------------------------
// alert_init
// ----------------------------------------------------------------
void alert_init(void) {
    // Enable all alerts (set enable bits for the first 32 alert sources).
    alert_write(ALERT_EN_0, 0xFFFFFFFFu);

    // ---- Class A: L1 -- NMI ----
    // Escalate on the first alert; fire NMI after 5 s.
    configure_class(ALERT_CLASS_A,
                    /* accum_thresh */ 1u,
                    /* timeout_cyc  */ 0u,
                    /* phase0_cyc   */ ESC_L1_CYCLES);

    // ---- Class B: L2 -- Reset ----
    // Escalate on the first alert; reset after 10 s.
    configure_class(ALERT_CLASS_B,
                    /* accum_thresh */ 1u,
                    /* timeout_cyc  */ 0u,
                    /* phase0_cyc   */ ESC_L2_CYCLES);

    // ---- Class C: L3 -- Zeroize ----
    // Escalate on the first alert; zeroize after 15 s.
    configure_class(ALERT_CLASS_C,
                    /* accum_thresh */ 1u,
                    /* timeout_cyc  */ 0u,
                    /* phase0_cyc   */ ESC_L3_CYCLES);

    // ---- Class D: L4 -- SCRAP (permanent) ----
    // Escalate on the first alert; no phase delay -- the lifecycle
    // controller transitions to SCRAP immediately.
    configure_class(ALERT_CLASS_D,
                    /* accum_thresh */ 1u,
                    /* timeout_cyc  */ 0u,
                    /* phase0_cyc   */ 0u);
}
