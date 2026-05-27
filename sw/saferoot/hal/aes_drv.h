// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// aes_drv.h — AES-256 hardware accelerator driver for SafeRoot

#ifndef SAFEROOT_AES_DRV_H_
#define SAFEROOT_AES_DRV_H_

#include "hal.h"

// AES operation mode
typedef enum {
    kAesEncrypt = 0x01,  // AES_ENC (2'b01)
    kAesDecrypt = 0x02,  // AES_DEC (2'b10)
} aes_op_t;

// AES block cipher mode (one-hot encoded, 6 bits)
typedef enum {
    kAesModeEcb = 0x01,  // ECB
    kAesModeCbc = 0x02,  // CBC
    kAesModeCfb = 0x04,  // CFB-128
    kAesModeOfb = 0x08,  // OFB
    kAesModeCtr = 0x10,  // CTR
} aes_mode_t;

// Initialize AES for a given operation and mode.
// key: pointer to 8 x uint32_t (256-bit key).
// iv:  pointer to 4 x uint32_t (128-bit IV), NULL for ECB mode.
hal_status_t aes_init(aes_op_t op, aes_mode_t mode,
                      const uint32_t key[8], const uint32_t *iv);

// Encrypt or decrypt a single 128-bit block (4 x uint32_t).
// Uses automatic mode (writes data_in, polls output_valid, reads data_out).
hal_status_t aes_process_block(const uint32_t data_in[4],
                               uint32_t data_out[4]);

// Clear key, IV, and data registers with pseudo-random values.
void aes_clear(void);

// Check if AES unit is idle.
bool aes_is_idle(void);

#endif  // SAFEROOT_AES_DRV_H_
