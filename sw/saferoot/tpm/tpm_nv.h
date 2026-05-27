// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// tpm_nv.h — TPM 2.0 NV (Non-Volatile) storage for SafeRoot
// Provides SRAM-backed NV indices (persisted to flash on shutdown).

#ifndef SAFEROOT_TPM_NV_H_
#define SAFEROOT_TPM_NV_H_

#include "tpm.h"

// Initialize NV storage (clear all indices)
void tpm_nv_init(void);

// Read data from an NV index.
// index: NV index number (0..TPM_NV_MAX_INDEX-1)
// data:  output buffer
// size:  number of bytes to read (max TPM_NV_MAX_SIZE)
// Returns TPM_RC_SUCCESS, TPM_RC_NV_RANGE, or TPM_RC_NV_UNINITIALIZED.
uint32_t tpm_nv_read(uint32_t index, uint8_t *data, uint32_t size);

// Write data to an NV index.
// index: NV index number (0..TPM_NV_MAX_INDEX-1)
// data:  input buffer
// size:  number of bytes to write (max TPM_NV_MAX_SIZE)
// Returns TPM_RC_SUCCESS or TPM_RC_NV_RANGE.
uint32_t tpm_nv_write(uint32_t index, const uint8_t *data, uint32_t size);

#endif  // SAFEROOT_TPM_NV_H_
