// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// sphincs_hash.h — Hash abstractions for SPHINCS+ (SHA-256 instantiation)
//
// All functions route through the HMAC HW accelerator via sha256().
// Reference implementation — not yet optimized for throughput.

#ifndef SAFEROOT_SPHINCS_HASH_H_
#define SAFEROOT_SPHINCS_HASH_H_

#include <stdint.h>
#include "sphincs_plus.h"

// ---------------------------------------------------------------------------
// Address structure for domain separation (SPHINCS+ ADRS)
// ---------------------------------------------------------------------------
typedef struct {
    uint32_t layer;        // Hypertree layer
    uint64_t tree;         // Tree index within layer
    uint32_t type;         // Address type (WOTS / FORS / tree)
    uint32_t keypair;      // Key pair index
    uint32_t chain;        // Chain index (WOTS)
    uint32_t hash;         // Hash index within chain
    uint32_t tree_height;  // Height in Merkle tree
    uint32_t tree_index;   // Index at that height
} sphincs_adrs_t;

// Address types
#define SPHINCS_ADRS_WOTS   0
#define SPHINCS_ADRS_TREE   1
#define SPHINCS_ADRS_FORS   2

// ---------------------------------------------------------------------------
// Hash functions
// ---------------------------------------------------------------------------

// Generic SHA-256 hash wrapper (truncated to SPHINCS_N bytes).
void sphincs_hash(const uint8_t *in, uint32_t in_len, uint8_t *out);

// PRF: keyed pseudorandom function.
// out = SHA-256(key ‖ adrs ‖ opt)[0..N-1]
void sphincs_prf(const uint8_t *key, const sphincs_adrs_t *adrs,
                 uint8_t *out);

// PRF for randomized signing.
// out = SHA-256(sk_prf ‖ opt_rand ‖ msg)[0..N-1]
void sphincs_prf_msg(const uint8_t *sk_prf, const uint8_t *opt_rand,
                     const uint8_t *msg, uint32_t msg_len, uint8_t *out);

// Tweakable hash F: single-block input.
// out = SHA-256(pk_seed ‖ adrs ‖ in)[0..N-1]
void sphincs_thash_f(const uint8_t *pk_seed, const sphincs_adrs_t *adrs,
                     const uint8_t *in, uint8_t *out);

// Tweakable hash H: two-block input (used in Merkle trees).
// out = SHA-256(pk_seed ‖ adrs ‖ in0 ‖ in1)[0..N-1]
void sphincs_thash_h(const uint8_t *pk_seed, const sphincs_adrs_t *adrs,
                     const uint8_t *in0, const uint8_t *in1, uint8_t *out);

// Message hash: H_msg(R, PK, M) — produces digest for FORS+HT addressing.
// out: buffer of sufficient length for FORS message digest
void sphincs_h_msg(const uint8_t *r, const uint8_t *pk,
                   const uint8_t *msg, uint32_t msg_len, uint8_t *out);

#endif  // SAFEROOT_SPHINCS_HASH_H_
