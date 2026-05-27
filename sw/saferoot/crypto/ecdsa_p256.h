// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// ecdsa_p256.h — ECDSA P-256 (secp256r1/prime256v1) software implementation
//
// Software fallback for ECDSA operations until OTBN coprocessor is integrated.
// Based on micro-ecc (public domain) approach, adapted for bare-metal rv32imc.
//
// Key sizes:
//   Private key:  32 bytes
//   Public key:   64 bytes (uncompressed x || y, no 0x04 prefix)
//   Signature:    64 bytes (r || s)
//   Hash:         32 bytes (SHA-256 digest)

#ifndef SAFEROOT_ECDSA_P256_H_
#define SAFEROOT_ECDSA_P256_H_

#include <stdint.h>

// Key and signature sizes in bytes
#define ECDSA_P256_PRIVKEY_BYTES  32U
#define ECDSA_P256_PUBKEY_BYTES   64U
#define ECDSA_P256_SIG_BYTES      64U
#define ECDSA_P256_HASH_BYTES     32U

// ---------------------------------------------------------------------------
// ECDSA operations
// ---------------------------------------------------------------------------

// Sign a 32-byte hash using a 32-byte private key.
// Produces a 64-byte signature (r || s) in big-endian.
// Returns 0 on success, non-zero on error.
int ecdsa_p256_sign(const uint8_t *private_key,
                    const uint8_t *hash,
                    uint8_t *signature);

// Verify a 64-byte signature against a 32-byte hash using a 64-byte public key.
// Returns 0 if the signature is valid, non-zero if invalid.
int ecdsa_p256_verify(const uint8_t *public_key,
                      const uint8_t *hash,
                      const uint8_t *signature);

// Generate an ECDSA-P256 key pair using CSRNG hardware.
// private_key: output 32-byte private key
// public_key:  output 64-byte public key (x || y)
// Returns 0 on success, non-zero on error.
int ecdsa_p256_keygen(uint8_t *private_key, uint8_t *public_key);

// Derive a 64-byte public key from a 32-byte private key.
// Returns 0 on success, non-zero on error.
int ecdsa_p256_compute_public(const uint8_t *private_key,
                              uint8_t *public_key);

#endif  // SAFEROOT_ECDSA_P256_H_
