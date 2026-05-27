// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// hmac_drv.h — HMAC-SHA256 hardware accelerator driver for SafeRoot

#ifndef SAFEROOT_HMAC_DRV_H_
#define SAFEROOT_HMAC_DRV_H_

#include "hal.h"

// Initialize HMAC-SHA256 with a 256-bit key.
// key: pointer to 8 x uint32_t. Pass NULL for plain SHA-256 (no HMAC).
hal_status_t hmac_init(const uint32_t *key);

// Feed data into the HMAC message FIFO.
// data must be 4-byte aligned, len in bytes (must be multiple of 4).
hal_status_t hmac_update(const void *data, uint32_t len);

// Signal end of message and retrieve the 256-bit digest.
// digest: pointer to 8 x uint32_t to receive result.
hal_status_t hmac_final(uint32_t digest[8]);

// Convenience: compute SHA-256 hash of a buffer in one call.
hal_status_t sha256(const void *data, uint32_t len, uint32_t digest[8]);

// Wipe internal state with a random value.
void hmac_wipe(uint32_t wipe_val);

#endif  // SAFEROOT_HMAC_DRV_H_
