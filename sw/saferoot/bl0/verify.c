// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// verify.c — Firmware image verification: hash, signature, anti-rollback
//
// Hash verification uses the HMAC/SHA-256 hardware accelerator.
// ECDSA-P256 signature verification is currently a stub (TODO).

#include "verify.h"
#include "../hal/hal.h"
#include "../hal/hmac_drv.h"
#include "../hal/otp_drv.h"
#include "../hal/uart_drv.h"

// ---------------------------------------------------------------------------
// Internal: verify SHA-256 hash of image payload against header
// ---------------------------------------------------------------------------
static bl0_error_t verify_hash(const fw_image_header_t *hdr,
                               const uint8_t *image,
                               uint32_t image_size)
{
    uint32_t computed_hash[SHA256_DIGEST_WORDS];
    hal_status_t st;

    // Compute SHA-256 of the image payload
    st = sha256(image, image_size, computed_hash);
    if (st != kHalOk) {
        uart_puts("BL0: SHA-256 computation failed\r\n");
        return kBl0ErrHashMismatch;
    }

    // Compare with hash stored in header (constant-time comparison)
    uint32_t diff = 0;
    for (uint32_t i = 0; i < SHA256_DIGEST_WORDS; i++) {
        diff |= computed_hash[i] ^ hdr->image_hash[i];
    }

    if (diff != 0) {
        uart_puts("BL0: Hash mismatch!\r\n");
        return kBl0ErrHashMismatch;
    }

    uart_puts("BL0: Hash OK\r\n");
    return kBl0Ok;
}

// ---------------------------------------------------------------------------
// Internal: verify ECDSA-P256 signature
// TODO: Implement actual ECDSA-P256 verification using crypto accelerator.
//       Currently returns OK unconditionally as a placeholder.
// ---------------------------------------------------------------------------
static bl0_error_t verify_signature(const fw_image_header_t *hdr,
                                    const uint32_t *hash)
{
    (void)hdr;
    (void)hash;

    // TODO(security): Implement ECDSA-P256 signature verification.
    // This requires:
    //   1. Public key loaded from OTP or ROM_EXT manifest
    //   2. OTBN co-processor for modular arithmetic
    //   3. P-256 curve point operations
    //
    // For now, this is a STUB that always returns success.
    // DO NOT ship this in production without implementing real verification.

    uart_puts("BL0: Sig verify STUB — OK (TODO: implement ECDSA)\r\n");
    return kBl0Ok;
}

// ---------------------------------------------------------------------------
// Anti-rollback check: image SVN must be >= OTP minimum SVN
// ---------------------------------------------------------------------------
bl0_error_t verify_antirollback(uint32_t image_svn)
{
    uint32_t min_svn = 0;
    hal_status_t st;

    // Read minimum SVN from OTP CreatorSwCfg partition
    st = otp_read_word(OTP_BL1_MIN_SVN_ADDR, &min_svn);
    if (st != kHalOk) {
        uart_puts("BL0: OTP read min_svn failed\r\n");
        return kBl0ErrOtpRead;
    }

    uart_puts("BL0: OTP min_svn=0x");
    uart_put_hex(min_svn);
    uart_puts(" image_svn=0x");
    uart_put_hex(image_svn);
    uart_puts("\r\n");

    if (image_svn < min_svn) {
        uart_puts("BL0: Anti-rollback FAIL\r\n");
        return kBl0ErrAntiRollback;
    }

    uart_puts("BL0: Anti-rollback OK\r\n");
    return kBl0Ok;
}

// ---------------------------------------------------------------------------
// Full image verification: hash + signature + anti-rollback
// ---------------------------------------------------------------------------
bl0_error_t verify_image(const fw_image_header_t *hdr,
                         const uint8_t *image,
                         uint32_t image_size)
{
    bl0_error_t err;

    // Step 1: Verify hash integrity
    err = verify_hash(hdr, image, image_size);
    if (err != kBl0Ok) {
        return err;
    }

    // Step 2: Verify ECDSA-P256 signature
    err = verify_signature(hdr, hdr->image_hash);
    if (err != kBl0Ok) {
        return err;
    }

    // Step 3: Anti-rollback SVN check
    err = verify_antirollback(hdr->security_version);
    if (err != kBl0Ok) {
        return err;
    }

    return kBl0Ok;
}
