// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// tpm_pcr.h — PCR (Platform Configuration Register) management for SafeRoot TPM

#ifndef SAFEROOT_TPM_PCR_H_
#define SAFEROOT_TPM_PCR_H_

#include "tpm.h"

// Initialize all PCR banks to zero
void tpm_pcr_init(void);

// Extend a PCR: PCR_new = SHA-256(PCR_old || digest)
// pcr_idx: PCR index (0..NUM_PCRS-1)
// digest:  32-byte SHA-256 digest to extend with
// Returns TPM_RC_SUCCESS or TPM_RC_VALUE on invalid index.
uint32_t tpm_pcr_extend(uint32_t pcr_idx, const uint8_t *digest);

// Read a PCR value
// pcr_idx: PCR index (0..NUM_PCRS-1)
// value:   output buffer, must be at least TPM_PCR_SIZE bytes
// Returns TPM_RC_SUCCESS or TPM_RC_VALUE on invalid index.
uint32_t tpm_pcr_read(uint32_t pcr_idx, uint8_t *value);

// Reset a PCR to all zeros.
// Only PCRs 16-23 are resettable at runtime per TCG spec.
// Returns TPM_RC_SUCCESS, TPM_RC_VALUE (bad idx), or TPM_RC_PCR (not resettable).
uint32_t tpm_pcr_reset(uint32_t pcr_idx);

// Restore a PCR value directly (used by Startup(STATE) to restore saved PCRs)
void tpm_pcr_restore(uint32_t pcr_idx, const uint8_t *value);

#endif  // SAFEROOT_TPM_PCR_H_
