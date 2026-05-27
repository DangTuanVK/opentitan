// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// verify.h — BL1 image signature verification and anti-rollback check

#ifndef SAFEROOT_VERIFY_H_
#define SAFEROOT_VERIFY_H_

#include <stdint.h>
#include "bl0.h"

// ---------------------------------------------------------------------------
// Verify the integrity and authenticity of a firmware image.
//
// Performs three checks:
//   1. Compute SHA-256 of image payload, compare with header hash
//   2. Verify ECDSA-P256 signature over the hash (TODO: stub)
//   3. Anti-rollback: image SVN >= minimum SVN from OTP
//
// hdr:        Pointer to the firmware image header
// image:      Pointer to the image payload (immediately after header)
// image_size: Size of the payload in bytes
//
// Returns kBl0Ok if all checks pass, or the appropriate error code.
// ---------------------------------------------------------------------------
bl0_error_t verify_image(const fw_image_header_t *hdr,
                         const uint8_t *image,
                         uint32_t image_size);

// ---------------------------------------------------------------------------
// Check anti-rollback SVN against OTP minimum.
//
// image_svn: Security Version Number from the image header
//
// Returns kBl0Ok if image_svn >= OTP minimum, kBl0ErrAntiRollback otherwise.
// ---------------------------------------------------------------------------
bl0_error_t verify_antirollback(uint32_t image_svn);

#endif  // SAFEROOT_VERIFY_H_
