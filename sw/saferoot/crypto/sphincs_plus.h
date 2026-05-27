// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// sphincs_plus.h — SPHINCS+-SHA256-128f post-quantum signature scheme
//
// Reference/stub implementation for correctness verification.
// NOT optimized for performance on Ibex — production version needs
// precomputation tables and OTBN offload (v2).
//
// NIST PQC standard: FIPS 205 (SLH-DSA)
// Parameter set: SPHINCS+-SHA256-128f (fast variant, 128-bit security)

#ifndef SAFEROOT_SPHINCS_PLUS_H_
#define SAFEROOT_SPHINCS_PLUS_H_

#include <stdint.h>

// ---------------------------------------------------------------------------
// SPHINCS+-SHA256-128f parameters
// ---------------------------------------------------------------------------
#define SPHINCS_N          16   // Security parameter (bytes)
#define SPHINCS_W          16   // Winternitz parameter
#define SPHINCS_H          60   // Total tree height
#define SPHINCS_D          20   // Number of hypertree layers
#define SPHINCS_K          14   // FORS trees
#define SPHINCS_A          12   // FORS tree height (log(t))
#define SPHINCS_TREE_H     (SPHINCS_H / SPHINCS_D)  // Per-layer height = 3

// Derived sizes
#define SPHINCS_WOTS_LEN1  (2 * SPHINCS_N)          // 32
#define SPHINCS_WOTS_LEN2  3                        // floor(log(len1*(w-1))/log(w))+1
#define SPHINCS_WOTS_LEN   (SPHINCS_WOTS_LEN1 + SPHINCS_WOTS_LEN2) // 35
#define SPHINCS_WOTS_SIG   (SPHINCS_WOTS_LEN * SPHINCS_N)          // 560

// Key and signature sizes (bytes)
#define SPHINCS_SK_BYTES   64    // SK.seed ‖ SK.prf ‖ PK.seed ‖ PK.root
#define SPHINCS_PK_BYTES   32    // PK.seed ‖ PK.root
#define SPHINCS_SIG_BYTES  7856  // R ‖ FORS sig ‖ HT sig

// ---------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------

// Generate a SPHINCS+ key pair.
// sk: output buffer, SPHINCS_SK_BYTES bytes
// pk: output buffer, SPHINCS_PK_BYTES bytes
// Returns 0 on success, non-zero on error.
int sphincs_keygen(uint8_t *sk, uint8_t *pk);

// Sign a message.
// sk:      secret key (SPHINCS_SK_BYTES bytes)
// msg:     message to sign
// msg_len: length of message in bytes
// sig:     output buffer (must be >= SPHINCS_SIG_BYTES)
// sig_len: receives actual signature length
// Returns 0 on success, non-zero on error.
int sphincs_sign(const uint8_t *sk, const uint8_t *msg, uint32_t msg_len,
                 uint8_t *sig, uint32_t *sig_len);

// Verify a signature.
// pk:      public key (SPHINCS_PK_BYTES bytes)
// msg:     message that was signed
// msg_len: length of message in bytes
// sig:     signature (SPHINCS_SIG_BYTES bytes)
// sig_len: length of signature in bytes
// Returns 0 if signature is VALID, non-zero otherwise.
int sphincs_verify(const uint8_t *pk, const uint8_t *msg, uint32_t msg_len,
                   const uint8_t *sig, uint32_t sig_len);

#endif  // SAFEROOT_SPHINCS_PLUS_H_
