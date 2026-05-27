// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// tpm_pcr.c — PCR bank implementation (SHA-256, 24 registers)

#include "tpm_pcr.h"
#include "../hal/hmac_drv.h"
#include "../hal/uart_drv.h"

// ---------------------------------------------------------------------------
// PCR storage: 24 banks x 32 bytes (SHA-256)
// ---------------------------------------------------------------------------
static uint8_t pcr_bank[TPM_NUM_PCRS][TPM_PCR_SIZE];

// ---------------------------------------------------------------------------
// tpm_pcr_init — Zero all PCRs
// ---------------------------------------------------------------------------
void tpm_pcr_init(void) {
    memset(pcr_bank, 0, sizeof(pcr_bank));
    uart_puts("[TPM] PCR banks initialized (24 x SHA-256)\r\n");
}

// ---------------------------------------------------------------------------
// tpm_pcr_extend — PCR_new = SHA-256(PCR_old || digest)
// ---------------------------------------------------------------------------
uint32_t tpm_pcr_extend(uint32_t pcr_idx, const uint8_t *digest) {
    if (pcr_idx >= TPM_NUM_PCRS) {
        return TPM_RC_VALUE;
    }
    if (digest == NULL) {
        return TPM_RC_VALUE;
    }

    // Concatenate: PCR_old (32 bytes) || digest (32 bytes) = 64 bytes
    // The HMAC hardware requires 4-byte aligned, 4-byte-multiple data.
    // 64 bytes satisfies both constraints.
    uint8_t concat[TPM_PCR_SIZE * 2] __attribute__((aligned(4)));
    memcpy(&concat[0], pcr_bank[pcr_idx], TPM_PCR_SIZE);
    memcpy(&concat[TPM_PCR_SIZE], digest, TPM_PCR_SIZE);

    // Compute SHA-256 of concatenation
    uint32_t hash[8];
    hal_status_t st = sha256(concat, sizeof(concat), hash);
    if (st != kHalOk) {
        return TPM_RC_FAILURE;
    }

    // Store result back into PCR (hash output is 8 x uint32_t = 32 bytes)
    memcpy(pcr_bank[pcr_idx], hash, TPM_PCR_SIZE);

    return TPM_RC_SUCCESS;
}

// ---------------------------------------------------------------------------
// tpm_pcr_read — Copy PCR value to caller buffer
// ---------------------------------------------------------------------------
uint32_t tpm_pcr_read(uint32_t pcr_idx, uint8_t *value) {
    if (pcr_idx >= TPM_NUM_PCRS) {
        return TPM_RC_VALUE;
    }
    if (value == NULL) {
        return TPM_RC_VALUE;
    }

    memcpy(value, pcr_bank[pcr_idx], TPM_PCR_SIZE);
    return TPM_RC_SUCCESS;
}

// ---------------------------------------------------------------------------
// tpm_pcr_reset — Reset a PCR to zero (only PCRs 16-23 at runtime)
// ---------------------------------------------------------------------------
uint32_t tpm_pcr_reset(uint32_t pcr_idx) {
    if (pcr_idx >= TPM_NUM_PCRS) {
        return TPM_RC_VALUE;
    }

    // Per TCG spec, only PCRs 16-23 are resettable after Startup
    if (pcr_idx < TPM_PCR_RESET_MIN) {
        return TPM_RC_PCR;
    }

    memset(pcr_bank[pcr_idx], 0, TPM_PCR_SIZE);
    return TPM_RC_SUCCESS;
}

void tpm_pcr_restore(uint32_t pcr_idx, const uint8_t *value) {
    if (pcr_idx < TPM_NUM_PCRS && value) {
        memcpy(pcr_bank[pcr_idx], value, TPM_PCR_SIZE);
    }
}
