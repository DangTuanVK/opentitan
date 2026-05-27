// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// sphincs_hash.c — SHA-256-based hash functions for SPHINCS+
//
// REFERENCE IMPLEMENTATION — routes all hashing through the HMAC HW
// accelerator (sha256()). Not optimized for throughput.
// PERF: optimize with precomputation/OTBN in v2

#include "sphincs_hash.h"
#include "../hal/hmac_drv.h"

#ifndef NULL
#define NULL ((void*)0)
#endif
extern void *memcpy(void *, const void *, unsigned int);
extern void *memset(void *, int, unsigned int);

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

// Serialize ADRS into a 32-byte buffer for domain separation.
static void adrs_to_bytes(const sphincs_adrs_t *adrs, uint8_t out[32]) {
    memset(out, 0, 32);
    // Pack fields into big-endian 32-byte block
    out[0]  = (uint8_t)(adrs->layer >> 24);
    out[1]  = (uint8_t)(adrs->layer >> 16);
    out[2]  = (uint8_t)(adrs->layer >> 8);
    out[3]  = (uint8_t)(adrs->layer);
    out[4]  = (uint8_t)(adrs->tree >> 56);
    out[5]  = (uint8_t)(adrs->tree >> 48);
    out[6]  = (uint8_t)(adrs->tree >> 40);
    out[7]  = (uint8_t)(adrs->tree >> 32);
    out[8]  = (uint8_t)(adrs->tree >> 24);
    out[9]  = (uint8_t)(adrs->tree >> 16);
    out[10] = (uint8_t)(adrs->tree >> 8);
    out[11] = (uint8_t)(adrs->tree);
    out[12] = (uint8_t)(adrs->type >> 24);
    out[13] = (uint8_t)(adrs->type >> 16);
    out[14] = (uint8_t)(adrs->type >> 8);
    out[15] = (uint8_t)(adrs->type);
    out[16] = (uint8_t)(adrs->keypair >> 24);
    out[17] = (uint8_t)(adrs->keypair >> 16);
    out[18] = (uint8_t)(adrs->keypair >> 8);
    out[19] = (uint8_t)(adrs->keypair);
    out[20] = (uint8_t)(adrs->chain >> 24);
    out[21] = (uint8_t)(adrs->chain >> 16);
    out[22] = (uint8_t)(adrs->chain >> 8);
    out[23] = (uint8_t)(adrs->chain);
    out[24] = (uint8_t)(adrs->hash >> 24);
    out[25] = (uint8_t)(adrs->hash >> 16);
    out[26] = (uint8_t)(adrs->hash >> 8);
    out[27] = (uint8_t)(adrs->hash);
    out[28] = (uint8_t)(adrs->tree_height >> 24);
    out[29] = (uint8_t)(adrs->tree_height >> 16);
    out[30] = (uint8_t)(adrs->tree_height >> 8);
    out[31] = (uint8_t)(adrs->tree_height);
}

// Call HW SHA-256 and truncate output to SPHINCS_N bytes.
// PERF: optimize with precomputation/OTBN in v2
static void sha256_truncated(const uint8_t *data, uint32_t len,
                             uint8_t out[SPHINCS_N]) {
    uint32_t digest[8];
    sha256(data, len, digest);
    // Copy first SPHINCS_N bytes (big-endian words -> byte array)
    for (uint32_t i = 0; i < SPHINCS_N / 4; i++) {
        out[i * 4 + 0] = (uint8_t)(digest[i] >> 24);
        out[i * 4 + 1] = (uint8_t)(digest[i] >> 16);
        out[i * 4 + 2] = (uint8_t)(digest[i] >> 8);
        out[i * 4 + 3] = (uint8_t)(digest[i]);
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void sphincs_hash(const uint8_t *in, uint32_t in_len, uint8_t *out) {
    sha256_truncated(in, in_len, out);
}

void sphincs_prf(const uint8_t *key, const sphincs_adrs_t *adrs,
                 uint8_t *out) {
    // PRF(key, adrs) = SHA-256(key ‖ adrs_bytes)[0..N-1]
    uint8_t buf[SPHINCS_N + 32];
    memcpy(buf, key, SPHINCS_N);
    adrs_to_bytes(adrs, buf + SPHINCS_N);
    sha256_truncated(buf, sizeof(buf), out);
}

void sphincs_prf_msg(const uint8_t *sk_prf, const uint8_t *opt_rand,
                     const uint8_t *msg, uint32_t msg_len, uint8_t *out) {
    // PRF_msg = SHA-256(sk_prf ‖ opt_rand ‖ msg)[0..N-1]
    // PERF: optimize with precomputation/OTBN in v2
    uint32_t total = SPHINCS_N + SPHINCS_N + msg_len;
    // Use incremental API for variable-length messages
    uint32_t digest[8];
    hmac_init(NULL);  // Plain SHA-256 mode
    hmac_update(sk_prf, SPHINCS_N);
    hmac_update(opt_rand, SPHINCS_N);
    hmac_update(msg, msg_len);
    hmac_final(digest);
    for (uint32_t i = 0; i < SPHINCS_N / 4; i++) {
        out[i * 4 + 0] = (uint8_t)(digest[i] >> 24);
        out[i * 4 + 1] = (uint8_t)(digest[i] >> 16);
        out[i * 4 + 2] = (uint8_t)(digest[i] >> 8);
        out[i * 4 + 3] = (uint8_t)(digest[i]);
    }
    (void)total;
}

void sphincs_thash_f(const uint8_t *pk_seed, const sphincs_adrs_t *adrs,
                     const uint8_t *in, uint8_t *out) {
    // F(pk_seed, adrs, in) = SHA-256(pk_seed ‖ adrs ‖ in)[0..N-1]
    uint8_t buf[SPHINCS_N + 32 + SPHINCS_N];
    memcpy(buf, pk_seed, SPHINCS_N);
    adrs_to_bytes(adrs, buf + SPHINCS_N);
    memcpy(buf + SPHINCS_N + 32, in, SPHINCS_N);
    sha256_truncated(buf, sizeof(buf), out);
}

void sphincs_thash_h(const uint8_t *pk_seed, const sphincs_adrs_t *adrs,
                     const uint8_t *in0, const uint8_t *in1, uint8_t *out) {
    // H(pk_seed, adrs, in0 ‖ in1) = SHA-256(pk_seed ‖ adrs ‖ in0 ‖ in1)[0..N-1]
    uint8_t buf[SPHINCS_N + 32 + 2 * SPHINCS_N];
    memcpy(buf, pk_seed, SPHINCS_N);
    adrs_to_bytes(adrs, buf + SPHINCS_N);
    memcpy(buf + SPHINCS_N + 32, in0, SPHINCS_N);
    memcpy(buf + SPHINCS_N + 32 + SPHINCS_N, in1, SPHINCS_N);
    sha256_truncated(buf, sizeof(buf), out);
}

void sphincs_h_msg(const uint8_t *r, const uint8_t *pk,
                   const uint8_t *msg, uint32_t msg_len, uint8_t *out) {
    // H_msg(R, PK, M) = SHA-256(R ‖ PK ‖ M)
    // Output: SPHINCS_K * SPHINCS_A bits for FORS + tree/leaf indices
    // PERF: optimize with precomputation/OTBN in v2
    uint32_t digest[8];
    hmac_init(NULL);
    hmac_update(r, SPHINCS_N);
    hmac_update(pk, SPHINCS_PK_BYTES);
    hmac_update(msg, msg_len);
    hmac_final(digest);
    // Copy full 32-byte digest — caller extracts needed bits
    for (uint32_t i = 0; i < 8; i++) {
        out[i * 4 + 0] = (uint8_t)(digest[i] >> 24);
        out[i * 4 + 1] = (uint8_t)(digest[i] >> 16);
        out[i * 4 + 2] = (uint8_t)(digest[i] >> 8);
        out[i * 4 + 3] = (uint8_t)(digest[i]);
    }
}
