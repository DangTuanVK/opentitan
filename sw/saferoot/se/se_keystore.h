// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// se_keystore.h — Key management for Secure Element

#ifndef SAFEROOT_SE_KEYSTORE_H_
#define SAFEROOT_SE_KEYSTORE_H_

#include <stdint.h>
#include "se.h"

// Initialize key store: load persistent keys from OTP/Flash into SRAM slots.
void keystore_init(void);

// Generate a new key in the given slot.
// slot: 0..SE_MAX_KEY_SLOTS-1, type: desired key algorithm.
// Returns SW_OK on success, SW_WRONG_DATA if slot is invalid.
uint16_t keystore_generate(uint8_t slot, se_key_type_t type);

// Retrieve key metadata and material for internal use.
// Returns SW_OK on success, SW_KEY_NOT_FOUND if slot empty.
uint16_t keystore_get(uint8_t slot, se_key_t *key);

// Securely delete (zeroize) a key slot.
// Returns SW_OK on success, SW_KEY_NOT_FOUND if slot empty.
uint16_t keystore_delete(uint8_t slot);

// Persist a key slot to flash (encrypted with OTP-derived wrapping key).
// Returns SW_OK on success.
uint16_t keystore_persist(uint8_t slot);

#endif  // SAFEROOT_SE_KEYSTORE_H_
