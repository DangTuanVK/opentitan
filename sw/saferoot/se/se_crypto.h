// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// se_crypto.h — Crypto operation interfaces for Secure Element

#ifndef SAFEROOT_SE_CRYPTO_H_
#define SAFEROOT_SE_CRYPTO_H_

#include <stdint.h>
#include "se.h"

// Generate cryptographically secure random bytes from CSRNG hardware.
// buf: destination buffer, len: number of bytes to generate.
// Returns SW_OK on success, SW_INTERNAL_ERROR on failure.
uint16_t se_get_random(uint8_t *buf, uint32_t len);

// AES-256-ECB encrypt using key material from the given slot.
// slot: key slot index, in: plaintext, out: ciphertext, len: byte count
//       (must be multiple of 16).
// Returns SW_OK on success.
uint16_t se_aes_encrypt(uint8_t slot, const uint8_t *in,
                        uint8_t *out, uint32_t len);

// AES-256-ECB decrypt using key material from the given slot.
// Returns SW_OK on success.
uint16_t se_aes_decrypt(uint8_t slot, const uint8_t *in,
                        uint8_t *out, uint32_t len);

// ECDSA-P256 sign a 32-byte hash using the private key in the given slot.
// hash: 32-byte SHA-256 digest, sig: output buffer (64 bytes: r || s).
// Returns SW_OK on success.
uint16_t se_sign(uint8_t slot, const uint8_t *hash, uint8_t *sig);

// ECDSA-P256 verify a signature against a 32-byte hash.
// hash: 32-byte digest, sig: 64-byte signature (r || s).
// Returns SW_OK if valid, SW_SECURITY if invalid.
uint16_t se_verify(uint8_t slot, const uint8_t *hash, const uint8_t *sig);

#endif  // SAFEROOT_SE_CRYPTO_H_
