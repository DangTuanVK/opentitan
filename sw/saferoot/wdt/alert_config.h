// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// Alert Handler escalation configuration for SafeRoot chip.
// Configures four escalation levels:
//   L1 = NMI           (5 s)
//   L2 = Reset          (10 s)
//   L3 = Zeroize secrets (15 s)
//   L4 = Lifecycle SCRAP (permanent)

#ifndef SAFEROOT_ALERT_CONFIG_H_
#define SAFEROOT_ALERT_CONFIG_H_

#include <stdint.h>

// ----------------------------------------------------------------
// Alert Handler base address (SafeRoot memory map)
// ----------------------------------------------------------------
#define ALERT_HANDLER_BASE  0x40110000u

// ----------------------------------------------------------------
// Alert Handler register offsets (simplified from OpenTitan)
// ----------------------------------------------------------------
#define ALERT_EN_0          0x00u  // Alert enable for class A
#define ALERT_CLASS_0       0x04u  // Alert-to-class mapping 0

// Per-class escalation registers (classes A..D map to L1..L4).
// Each class has a block of registers at CLASS_BASE + class_idx*0x20.
#define ALERT_CLASS_BASE    0x80u
#define ALERT_CLASS_STRIDE  0x20u

// Offsets within each class block:
#define CLASS_CTRL          0x00u  // Control: enable, lock, escalation enable
#define CLASS_CLR           0x04u  // Clear accumulator
#define CLASS_ACCUM_CNT     0x08u  // Accumulator count
#define CLASS_ACCUM_THRESH  0x0Cu  // Accumulator threshold for escalation
#define CLASS_TIMEOUT       0x10u  // IRQ timeout (cycles before escalation)
#define CLASS_PHASE0_CYC    0x14u  // Phase 0 duration (cycles)
#define CLASS_PHASE1_CYC    0x18u  // Phase 1 duration (cycles)
#define CLASS_PHASE2_CYC    0x1Cu  // Phase 2 duration (cycles)

// CLASS_CTRL bit positions
#define CLASS_CTRL_EN_BIT       0u
#define CLASS_CTRL_LOCK_BIT     1u
#define CLASS_CTRL_ESC_EN_BIT   2u

// ----------------------------------------------------------------
// Escalation timing (SafeRoot AON clock = 200 kHz)
// ----------------------------------------------------------------
#define AON_CLK_HZ          200000u

#define ESC_L1_NMI_SEC          5u
#define ESC_L2_RESET_SEC       10u
#define ESC_L3_ZEROIZE_SEC     15u
// L4 (SCRAP) has no duration -- it is permanent.

#define ESC_L1_CYCLES   (ESC_L1_NMI_SEC      * AON_CLK_HZ)  //  1 000 000
#define ESC_L2_CYCLES   (ESC_L2_RESET_SEC    * AON_CLK_HZ)  //  2 000 000
#define ESC_L3_CYCLES   (ESC_L3_ZEROIZE_SEC  * AON_CLK_HZ)  //  3 000 000

// ----------------------------------------------------------------
// Alert class indices
// ----------------------------------------------------------------
#define ALERT_CLASS_A  0u   // L1 -- NMI
#define ALERT_CLASS_B  1u   // L2 -- Reset
#define ALERT_CLASS_C  2u   // L3 -- Zeroize
#define ALERT_CLASS_D  3u   // L4 -- SCRAP

// ----------------------------------------------------------------
// Public API
// ----------------------------------------------------------------

/**
 * Initialise the Alert Handler with the SafeRoot four-level
 * escalation policy.
 */
void alert_init(void);

#endif  // SAFEROOT_ALERT_CONFIG_H_
