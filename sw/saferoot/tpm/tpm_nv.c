// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// tpm_nv.c — NV storage for SafeRoot TPM
// SRAM-backed, 16 indices x 256 bytes max each.

#include "tpm_nv.h"
#include "../hal/uart_drv.h"

// ---------------------------------------------------------------------------
// NV slot metadata
// ---------------------------------------------------------------------------
typedef struct {
    bool     defined;   // true if this NV index has been written at least once
    uint32_t size;      // current data size in bytes
} nv_meta_t;

// ---------------------------------------------------------------------------
// NV storage (SRAM)
// ---------------------------------------------------------------------------
static uint8_t   nv_data[TPM_NV_MAX_INDEX][TPM_NV_MAX_SIZE];
static nv_meta_t nv_meta[TPM_NV_MAX_INDEX];

// ---------------------------------------------------------------------------
// tpm_nv_init — Clear all NV indices
// ---------------------------------------------------------------------------
void tpm_nv_init(void) {
    memset(nv_data, 0, sizeof(nv_data));
    memset(nv_meta, 0, sizeof(nv_meta));
    uart_puts("[TPM] NV storage initialized (16 x 256B)\r\n");
}

// ---------------------------------------------------------------------------
// tpm_nv_read
// ---------------------------------------------------------------------------
uint32_t tpm_nv_read(uint32_t index, uint8_t *data, uint32_t size) {
    if (index >= TPM_NV_MAX_INDEX || size > TPM_NV_MAX_SIZE) {
        return TPM_RC_NV_RANGE;
    }
    if (data == NULL) {
        return TPM_RC_VALUE;
    }
    if (!nv_meta[index].defined) {
        return TPM_RC_NV_UNINITIALIZED;
    }
    if (size > nv_meta[index].size) {
        size = nv_meta[index].size;
    }

    memcpy(data, nv_data[index], size);
    return TPM_RC_SUCCESS;
}

// ---------------------------------------------------------------------------
// tpm_nv_write
// ---------------------------------------------------------------------------
uint32_t tpm_nv_write(uint32_t index, const uint8_t *data, uint32_t size) {
    if (index >= TPM_NV_MAX_INDEX || size > TPM_NV_MAX_SIZE) {
        return TPM_RC_NV_RANGE;
    }
    if (data == NULL) {
        return TPM_RC_VALUE;
    }

    memcpy(nv_data[index], data, size);
    nv_meta[index].defined = true;
    nv_meta[index].size    = size;

    return TPM_RC_SUCCESS;
}
