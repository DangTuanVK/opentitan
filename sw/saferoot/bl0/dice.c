// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// dice.c — DICE CDI derivation using HMAC-SHA256 hardware accelerator
//
// Implements TCG DICE Layered Architecture:
//   CDI_0 = HMAC-SHA256(UDS, Hash(BL1) || "SafeRoot.BL0.CDI_0")
//
// The UDS (Unique Device Secret) is read from OTP Secret2 partition
// by the caller and passed as prev_secret.

#include "dice.h"
#include "../hal/hal.h"
#include "../hal/hmac_drv.h"
#include "../hal/uart_drv.h"

// ---------------------------------------------------------------------------
// Internal: build the HMAC message = fw_hash || boot_descriptor
// We feed it in two hmac_update() calls to avoid a temporary copy.
// ---------------------------------------------------------------------------

// Align descriptor to 4 bytes for HMAC FIFO requirement.
// Pad with zeros to reach next 4-byte boundary.
static const uint8_t dice_descriptor_padded[20] __attribute__((aligned(4))) = {
    // "SafeRoot.BL0.CDI_0" = 18 bytes + 2 bytes zero padding
    'S','a','f','e','R','o','o','t','.','B','L','0','.','C','D','I','_','0',
    0x00, 0x00
};

bl0_error_t dice_derive_cdi(const uint8_t *prev_secret,
                            const uint8_t *fw_hash,
                            uint8_t *cdi_out)
{
    hal_status_t st;

    // prev_secret is 32 bytes = 8 x uint32_t, used as HMAC key.
    // HMAC-SHA256(key, message) where key = prev_secret.
    st = hmac_init((const uint32_t *)prev_secret);
    if (st != kHalOk) {
        uart_puts("BL0: DICE hmac_init failed\r\n");
        return kBl0ErrDice;
    }

    // Feed fw_hash (32 bytes, already 4-byte aligned from SHA-256 output)
    st = hmac_update(fw_hash, SHA256_DIGEST_BYTES);
    if (st != kHalOk) {
        uart_puts("BL0: DICE hmac_update(hash) failed\r\n");
        hmac_wipe(0x0);
        return kBl0ErrDice;
    }

    // Feed boot descriptor (padded to 20 bytes for 4-byte alignment).
    // The HMAC processes DICE_BOOT_DESCRIPTOR_LEN meaningful bytes;
    // the extra 2 zero-pad bytes are included to satisfy the FIFO
    // alignment constraint. This is deterministic and reproducible.
    st = hmac_update(dice_descriptor_padded, 20U);
    if (st != kHalOk) {
        uart_puts("BL0: DICE hmac_update(desc) failed\r\n");
        hmac_wipe(0x0);
        return kBl0ErrDice;
    }

    // Finalize: read 256-bit digest = CDI
    uint32_t digest[SHA256_DIGEST_WORDS];
    st = hmac_final(digest);
    if (st != kHalOk) {
        uart_puts("BL0: DICE hmac_final failed\r\n");
        hmac_wipe(0x0);
        return kBl0ErrDice;
    }

    // Copy digest to cdi_out (byte-level copy for portability)
    const uint8_t *src = (const uint8_t *)digest;
    for (uint32_t i = 0; i < DICE_CDI_BYTES; i++) {
        cdi_out[i] = src[i];
    }

    // Wipe digest from stack
    for (uint32_t i = 0; i < SHA256_DIGEST_WORDS; i++) {
        digest[i] = 0U;
    }
    HAL_BARRIER();

    // Wipe HMAC engine internal state
    hmac_wipe(0xA5A5A5A5);

    uart_puts("BL0: DICE CDI derived OK\r\n");
    return kBl0Ok;
}

void dice_wipe_secrets(dice_cdi_t *cdi)
{
    if (cdi == (void *)0) {
        return;
    }

    // Zero CDI bytes
    for (uint32_t i = 0; i < DICE_CDI_BYTES; i++) {
        cdi->cdi[i] = 0U;
    }
    cdi->flags = 0U;

    // Ensure wipe is not optimized away
    HAL_BARRIER();
}
