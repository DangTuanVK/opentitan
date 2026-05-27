// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// dice.h — DICE (Device Identifier Composition Engine) key derivation
// Implements TCG DICE Layered Architecture for measured boot.

#ifndef SAFEROOT_DICE_H_
#define SAFEROOT_DICE_H_

#include <stdint.h>
#include "bl0.h"

// DICE boot descriptor string (bound into CDI derivation)
#define DICE_BOOT_DESCRIPTOR    "SafeRoot.BL0.CDI_0"
#define DICE_BOOT_DESCRIPTOR_LEN 18U

// ---------------------------------------------------------------------------
// Derive a Compound Device Identifier (CDI).
//
// CDI = HMAC-SHA256(prev_secret, fw_hash || boot_descriptor)
//
// prev_secret: 32-byte secret from previous layer (UDS for CDI_0)
// fw_hash:     32-byte SHA-256 hash of the next firmware image
// cdi_out:     32-byte output CDI
//
// Returns kBl0Ok on success, kBl0ErrDice on failure.
// ---------------------------------------------------------------------------
bl0_error_t dice_derive_cdi(const uint8_t *prev_secret,
                            const uint8_t *fw_hash,
                            uint8_t *cdi_out);

// ---------------------------------------------------------------------------
// Wipe DICE secrets from memory.
// Zeroes the CDI buffer and issues a memory barrier.
// ---------------------------------------------------------------------------
void dice_wipe_secrets(dice_cdi_t *cdi);

#endif  // SAFEROOT_DICE_H_
