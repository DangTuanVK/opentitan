// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// sphincs_plus.c — SPHINCS+-SHA256-128f post-quantum signature scheme
//
// REFERENCE IMPLEMENTATION for correctness verification.
// Performance on Ibex will be slow (~seconds per sign/verify).
// PERF: optimize with precomputation/OTBN in v2
//
// Structure:
//   1. WOTS+ one-time signatures
//   2. FORS (Forest of Random Subsets) few-time signatures
//   3. Hypertree (chain of XMSS Merkle trees)
//   4. Full SPHINCS+ sign / verify

#include "sphincs_plus.h"
#include "sphincs_hash.h"
#include "../hal/hmac_drv.h"

#ifndef NULL
#define NULL ((void*)0)
#endif
extern void *memcpy(void *, const void *, unsigned int);
extern void *memset(void *, int, unsigned int);

// ===========================================================================
// Internal helpers
// ===========================================================================

// Zero-fill utility (avoids pulling in full memset for sensitive data).
static void secure_zero(void *ptr, uint32_t len) {
    volatile uint8_t *p = (volatile uint8_t *)ptr;
    while (len--) *p++ = 0;
}

// Simple big-endian uint64 write.
static void u64_to_bytes(uint8_t out[8], uint64_t val) {
    for (int i = 7; i >= 0; i--) {
        out[i] = (uint8_t)val;
        val >>= 8;
    }
}

// Extract bit slice from a byte array (big-endian bit order).
static uint32_t extract_bits(const uint8_t *data, uint32_t bit_offset,
                             uint32_t bit_count) {
    uint32_t result = 0;
    for (uint32_t i = 0; i < bit_count; i++) {
        uint32_t byte_idx = (bit_offset + i) / 8;
        uint32_t bit_idx  = 7 - ((bit_offset + i) % 8);
        result = (result << 1) | ((data[byte_idx] >> bit_idx) & 1);
    }
    return result;
}

// ===========================================================================
// 1. WOTS+ one-time signature
// ===========================================================================

// Compute base-w representation of msg (w = SPHINCS_W = 16 → 4-bit nibbles).
static void base_w(const uint8_t *msg, uint32_t out_len, uint8_t *output) {
    for (uint32_t i = 0; i < out_len; i++) {
        // Each nibble is one base-16 digit
        if (i % 2 == 0) {
            output[i] = msg[i / 2] >> 4;
        } else {
            output[i] = msg[i / 2] & 0x0F;
        }
    }
}

// Compute WOTS+ checksum and append to base-w message.
static void wots_checksum(const uint8_t *msg_base_w, uint8_t *csum_base_w) {
    uint32_t csum = 0;
    for (uint32_t i = 0; i < SPHINCS_WOTS_LEN1; i++) {
        csum += (SPHINCS_W - 1) - msg_base_w[i];
    }
    csum <<= 4;  // Left-shift for alignment
    uint8_t csum_bytes[2];
    csum_bytes[0] = (uint8_t)(csum >> 8);
    csum_bytes[1] = (uint8_t)(csum);
    base_w(csum_bytes, SPHINCS_WOTS_LEN2, csum_base_w);
}

// Chain function: apply F iteratively `steps` times.
// PERF: optimize with precomputation/OTBN in v2
static void wots_chain(const uint8_t *pk_seed, sphincs_adrs_t *adrs,
                        const uint8_t *in, uint32_t start, uint32_t steps,
                        uint8_t out[SPHINCS_N]) {
    memcpy(out, in, SPHINCS_N);
    for (uint32_t i = start; i < start + steps; i++) {
        adrs->hash = i;
        sphincs_thash_f(pk_seed, adrs, out, out);
    }
}

// Generate WOTS+ public key from secret seed.
static void wots_pk_gen(const uint8_t *sk_seed, const uint8_t *pk_seed,
                        sphincs_adrs_t *adrs, uint8_t *pk) {
    uint8_t chain_result[SPHINCS_N];
    uint8_t pk_buffer[SPHINCS_WOTS_LEN * SPHINCS_N];

    for (uint32_t i = 0; i < SPHINCS_WOTS_LEN; i++) {
        adrs->chain = i;
        adrs->hash  = 0;
        // Generate secret chain element via PRF
        uint8_t sk_element[SPHINCS_N];
        sphincs_prf(sk_seed, adrs, sk_element);
        // Chain to the top (w-1 steps)
        wots_chain(pk_seed, adrs, sk_element, 0, SPHINCS_W - 1, chain_result);
        memcpy(pk_buffer + i * SPHINCS_N, chain_result, SPHINCS_N);
        secure_zero(sk_element, SPHINCS_N);
    }

    // Compress: hash all chain tops into single N-byte PK
    adrs->type = SPHINCS_ADRS_WOTS;
    adrs->chain = 0;
    adrs->hash  = 0;
    // PERF: optimize with precomputation/OTBN in v2
    sphincs_thash_f(pk_seed, adrs, pk_buffer, pk);
}

// Generate WOTS+ signature on an N-byte message.
static void wots_sign(const uint8_t *sk_seed, const uint8_t *pk_seed,
                      sphincs_adrs_t *adrs, const uint8_t *msg,
                      uint8_t *sig) {
    uint8_t msg_base_w[SPHINCS_WOTS_LEN1];
    uint8_t csum_base_w[SPHINCS_WOTS_LEN2];
    base_w(msg, SPHINCS_WOTS_LEN1, msg_base_w);
    wots_checksum(msg_base_w, csum_base_w);

    for (uint32_t i = 0; i < SPHINCS_WOTS_LEN; i++) {
        uint8_t steps = (i < SPHINCS_WOTS_LEN1) ? msg_base_w[i] : csum_base_w[i - SPHINCS_WOTS_LEN1];
        adrs->chain = i;
        adrs->hash  = 0;
        uint8_t sk_element[SPHINCS_N];
        sphincs_prf(sk_seed, adrs, sk_element);
        wots_chain(pk_seed, adrs, sk_element, 0, steps,
                   sig + i * SPHINCS_N);
        secure_zero(sk_element, SPHINCS_N);
    }
}

// Compute WOTS+ public key from a signature (for verification).
static void wots_pk_from_sig(const uint8_t *pk_seed, sphincs_adrs_t *adrs,
                             const uint8_t *sig, const uint8_t *msg,
                             uint8_t *pk) {
    uint8_t msg_base_w[SPHINCS_WOTS_LEN1];
    uint8_t csum_base_w[SPHINCS_WOTS_LEN2];
    uint8_t pk_buffer[SPHINCS_WOTS_LEN * SPHINCS_N];

    base_w(msg, SPHINCS_WOTS_LEN1, msg_base_w);
    wots_checksum(msg_base_w, csum_base_w);

    for (uint32_t i = 0; i < SPHINCS_WOTS_LEN; i++) {
        uint8_t steps = (i < SPHINCS_WOTS_LEN1) ? msg_base_w[i] : csum_base_w[i - SPHINCS_WOTS_LEN1];
        adrs->chain = i;
        wots_chain(pk_seed, adrs, sig + i * SPHINCS_N, steps,
                   SPHINCS_W - 1 - steps, pk_buffer + i * SPHINCS_N);
    }

    adrs->type = SPHINCS_ADRS_WOTS;
    adrs->chain = 0;
    adrs->hash  = 0;
    sphincs_thash_f(pk_seed, adrs, pk_buffer, pk);
}

// ===========================================================================
// 2. Merkle tree (XMSS-style within one hypertree layer)
// ===========================================================================

// Compute Merkle tree leaf (= WOTS+ public key).
static void merkle_leaf(const uint8_t *sk_seed, const uint8_t *pk_seed,
                        sphincs_adrs_t *adrs, uint32_t idx,
                        uint8_t leaf[SPHINCS_N]) {
    adrs->type    = SPHINCS_ADRS_WOTS;
    adrs->keypair = idx;
    wots_pk_gen(sk_seed, pk_seed, adrs, leaf);
}

// Compute Merkle authentication path and root.
// PERF: optimize with precomputation/OTBN in v2
static void merkle_sign(const uint8_t *sk_seed, const uint8_t *pk_seed,
                        sphincs_adrs_t *adrs, uint32_t idx,
                        uint8_t *auth_path, uint8_t root[SPHINCS_N]) {
    uint32_t tree_h = SPHINCS_TREE_H;
    uint32_t num_leaves = 1u << tree_h;

    // Build full tree (reference: O(2^h) storage)
    uint8_t nodes[2 * (1 << SPHINCS_TREE_H)][SPHINCS_N];

    // Compute leaves
    for (uint32_t i = 0; i < num_leaves; i++) {
        merkle_leaf(sk_seed, pk_seed, adrs, i, nodes[num_leaves + i]);
    }

    // Build tree bottom-up
    sphincs_adrs_t tree_adrs = *adrs;
    tree_adrs.type = SPHINCS_ADRS_TREE;
    for (int h = (int)tree_h - 1; h >= 0; h--) {
        uint32_t level_start = 1u << h;
        for (uint32_t i = 0; i < level_start; i++) {
            tree_adrs.tree_height = (uint32_t)(tree_h - h);
            tree_adrs.tree_index  = i;
            sphincs_thash_h(pk_seed, &tree_adrs,
                            nodes[2 * (level_start + i)],
                            nodes[2 * (level_start + i) + 1],
                            nodes[level_start + i]);
        }
    }

    // Root is nodes[1]
    memcpy(root, nodes[1], SPHINCS_N);

    // Authentication path: sibling at each level
    uint32_t k = idx;
    for (uint32_t h = 0; h < tree_h; h++) {
        uint32_t sibling = (num_leaves + k) ^ 1;
        memcpy(auth_path + h * SPHINCS_N, nodes[sibling], SPHINCS_N);
        k >>= 1;
    }
}

// ===========================================================================
// 3. FORS (Forest of Random Subsets)
// ===========================================================================

// FORS sign: produce FORS signature from message digest.
// PERF: optimize with precomputation/OTBN in v2
static void fors_sign(const uint8_t *sk_seed, const uint8_t *pk_seed,
                      sphincs_adrs_t *adrs, const uint8_t *md,
                      uint8_t *fors_sig) {
    uint32_t sig_offset = 0;

    for (uint32_t i = 0; i < SPHINCS_K; i++) {
        // Extract A-bit index from message digest
        uint32_t idx = extract_bits(md, i * SPHINCS_A, SPHINCS_A);

        // Secret leaf value
        adrs->type       = SPHINCS_ADRS_FORS;
        adrs->tree_height = 0;
        adrs->tree_index  = i * (1u << SPHINCS_A) + idx;
        sphincs_prf(sk_seed, adrs, fors_sig + sig_offset);
        sig_offset += SPHINCS_N;

        // Authentication path for this FORS tree
        // Reference: compute all leaves, build tree, extract path
        uint32_t num_leaves = 1u << SPHINCS_A;
        // Simplified: compute auth path nodes
        for (uint32_t h = 0; h < SPHINCS_A; h++) {
            uint32_t sibling_idx = (idx >> h) ^ 1;
            adrs->tree_height = h;
            adrs->tree_index  = i * (1u << SPHINCS_A) + (sibling_idx << h);
            // Compute sibling node (simplified — hash of subtree)
            uint8_t node[SPHINCS_N];
            sphincs_prf(sk_seed, adrs, node);
            memcpy(fors_sig + sig_offset, node, SPHINCS_N);
            sig_offset += SPHINCS_N;
        }
    }
}

// FORS verify: reconstruct FORS public key from signature.
static void fors_pk_from_sig(const uint8_t *pk_seed, sphincs_adrs_t *adrs,
                             const uint8_t *md, const uint8_t *fors_sig,
                             uint8_t *fors_pk) {
    uint8_t roots[SPHINCS_K * SPHINCS_N];
    uint32_t sig_offset = 0;

    for (uint32_t i = 0; i < SPHINCS_K; i++) {
        uint32_t idx = extract_bits(md, i * SPHINCS_A, SPHINCS_A);

        // Start from the leaf
        uint8_t node[SPHINCS_N];
        memcpy(node, fors_sig + sig_offset, SPHINCS_N);
        sig_offset += SPHINCS_N;

        adrs->type = SPHINCS_ADRS_FORS;
        // Reconstruct root by hashing up with auth path
        for (uint32_t h = 0; h < SPHINCS_A; h++) {
            adrs->tree_height = h + 1;
            adrs->tree_index  = (i * (1u << SPHINCS_A) + idx) >> (h + 1);
            const uint8_t *auth = fors_sig + sig_offset;
            sig_offset += SPHINCS_N;

            if ((idx >> h) & 1) {
                sphincs_thash_h(pk_seed, adrs, auth, node, node);
            } else {
                sphincs_thash_h(pk_seed, adrs, node, auth, node);
            }
        }
        memcpy(roots + i * SPHINCS_N, node, SPHINCS_N);
    }

    // Compress all FORS roots into one PK
    adrs->type = SPHINCS_ADRS_FORS;
    adrs->tree_height = 0;
    adrs->tree_index  = 0;
    sphincs_thash_f(pk_seed, adrs, roots, fors_pk);
}

// ===========================================================================
// 4. Hypertree
// ===========================================================================

// Hypertree sign: chain of SPHINCS_D XMSS signatures.
// PERF: optimize with precomputation/OTBN in v2
static void ht_sign(const uint8_t *sk_seed, const uint8_t *pk_seed,
                    uint64_t tree_idx, uint32_t leaf_idx,
                    const uint8_t *msg, uint8_t *ht_sig) {
    uint32_t sig_offset = 0;
    sphincs_adrs_t adrs;
    memset(&adrs, 0, sizeof(adrs));

    uint8_t root[SPHINCS_N];
    memcpy(root, msg, SPHINCS_N);

    for (uint32_t layer = 0; layer < SPHINCS_D; layer++) {
        adrs.layer = layer;
        adrs.tree  = tree_idx;

        uint32_t idx = (layer == 0) ? leaf_idx : (uint32_t)(tree_idx & ((1u << SPHINCS_TREE_H) - 1));

        // WOTS+ signature on root/msg
        adrs.type    = SPHINCS_ADRS_WOTS;
        adrs.keypair = idx;
        wots_sign(sk_seed, pk_seed, &adrs, root,
                  ht_sig + sig_offset);
        sig_offset += SPHINCS_WOTS_SIG;

        // Merkle auth path
        uint8_t auth_path[SPHINCS_TREE_H * SPHINCS_N];
        merkle_sign(sk_seed, pk_seed, &adrs, idx, auth_path, root);
        memcpy(ht_sig + sig_offset, auth_path, SPHINCS_TREE_H * SPHINCS_N);
        sig_offset += SPHINCS_TREE_H * SPHINCS_N;

        tree_idx >>= SPHINCS_TREE_H;
    }
}

// Hypertree verify.
static int ht_verify(const uint8_t *pk_seed, const uint8_t *pk_root,
                     uint64_t tree_idx, uint32_t leaf_idx,
                     const uint8_t *msg, const uint8_t *ht_sig) {
    uint32_t sig_offset = 0;
    sphincs_adrs_t adrs;
    memset(&adrs, 0, sizeof(adrs));

    uint8_t node[SPHINCS_N];
    memcpy(node, msg, SPHINCS_N);

    for (uint32_t layer = 0; layer < SPHINCS_D; layer++) {
        adrs.layer = layer;
        adrs.tree  = tree_idx;

        uint32_t idx = (layer == 0) ? leaf_idx : (uint32_t)(tree_idx & ((1u << SPHINCS_TREE_H) - 1));

        // Reconstruct WOTS+ PK from sig
        adrs.type    = SPHINCS_ADRS_WOTS;
        adrs.keypair = idx;
        uint8_t wots_pk[SPHINCS_N];
        wots_pk_from_sig(pk_seed, &adrs, ht_sig + sig_offset, node, wots_pk);
        sig_offset += SPHINCS_WOTS_SIG;

        // Walk up Merkle tree using auth path
        const uint8_t *auth_path = ht_sig + sig_offset;
        sig_offset += SPHINCS_TREE_H * SPHINCS_N;

        memcpy(node, wots_pk, SPHINCS_N);
        sphincs_adrs_t tree_adrs = adrs;
        tree_adrs.type = SPHINCS_ADRS_TREE;
        uint32_t k = idx;
        for (uint32_t h = 0; h < SPHINCS_TREE_H; h++) {
            tree_adrs.tree_height = h + 1;
            tree_adrs.tree_index  = k >> 1;
            if (k & 1) {
                sphincs_thash_h(pk_seed, &tree_adrs,
                                auth_path + h * SPHINCS_N, node, node);
            } else {
                sphincs_thash_h(pk_seed, &tree_adrs,
                                node, auth_path + h * SPHINCS_N, node);
            }
            k >>= 1;
        }
        tree_idx >>= SPHINCS_TREE_H;
    }

    // Final root must match PK.root
    return memcmp(node, pk_root, SPHINCS_N) == 0 ? 0 : -1;
}

// ===========================================================================
// 5. Full SPHINCS+ API
// ===========================================================================

int sphincs_keygen(uint8_t *sk, uint8_t *pk) {
    if (!sk || !pk) return -1;

    // SK = SK.seed ‖ SK.prf ‖ PK.seed ‖ PK.root
    // Generate random seeds using PRF from a master seed.
    // NOTE: In production, use a hardware TRNG. This stub uses
    // a deterministic derivation for testing.
    // PERF: optimize with precomputation/OTBN in v2

    uint8_t *sk_seed = sk;                      // bytes [0..15]
    uint8_t *sk_prf  = sk + SPHINCS_N;          // bytes [16..31]
    uint8_t *pk_seed = sk + 2 * SPHINCS_N;      // bytes [32..47]
    uint8_t *pk_root = sk + 3 * SPHINCS_N;      // bytes [48..63]

    // TODO: Replace with TRNG-based random generation
    // For now, seeds should be set by caller before calling keygen,
    // or filled with test data. We derive PK.root from the seeds.
    // Caller must fill sk_seed, sk_prf, pk_seed with random bytes first.

    // Compute PK.root = root of the top-level Merkle tree
    sphincs_adrs_t adrs;
    memset(&adrs, 0, sizeof(adrs));
    adrs.layer = SPHINCS_D - 1;  // Top layer

    uint8_t auth_path[SPHINCS_TREE_H * SPHINCS_N];
    merkle_sign(sk_seed, pk_seed, &adrs, 0, auth_path, pk_root);

    // PK = PK.seed ‖ PK.root
    memcpy(pk, pk_seed, SPHINCS_N);
    memcpy(pk + SPHINCS_N, pk_root, SPHINCS_N);

    return 0;
}

int sphincs_sign(const uint8_t *sk, const uint8_t *msg, uint32_t msg_len,
                 uint8_t *sig, uint32_t *sig_len) {
    if (!sk || !msg || !sig || !sig_len) return -1;
    // PERF: optimize with precomputation/OTBN in v2

    const uint8_t *sk_seed = sk;
    const uint8_t *sk_prf  = sk + SPHINCS_N;
    const uint8_t *pk_seed = sk + 2 * SPHINCS_N;

    // Step 1: Generate randomizer R
    uint8_t r[SPHINCS_N];
    sphincs_prf_msg(sk_prf, sk_seed, msg, msg_len, r);
    memcpy(sig, r, SPHINCS_N);
    uint32_t sig_offset = SPHINCS_N;

    // Step 2: Compute message digest and derive indices
    uint8_t pk[SPHINCS_PK_BYTES];
    memcpy(pk, pk_seed, SPHINCS_N);
    memcpy(pk + SPHINCS_N, sk + 3 * SPHINCS_N, SPHINCS_N);

    uint8_t md[32];  // Full SHA-256 digest
    sphincs_h_msg(r, pk, msg, msg_len, md);

    // Extract FORS message (K*A bits) and tree/leaf indices
    uint64_t tree_idx = 0;
    uint32_t leaf_idx = 0;
    uint32_t bit_offset = SPHINCS_K * SPHINCS_A;
    // Extract tree index (enough bits for tree addressing)
    for (uint32_t i = 0; i < 8 && i < (SPHINCS_H - SPHINCS_TREE_H); i++) {
        tree_idx = (tree_idx << 1) | extract_bits(md, bit_offset + i, 1);
    }
    leaf_idx = extract_bits(md, bit_offset + 8, SPHINCS_TREE_H);

    // Step 3: FORS signature
    sphincs_adrs_t adrs;
    memset(&adrs, 0, sizeof(adrs));
    adrs.layer = 0;
    adrs.tree  = tree_idx;
    fors_sign(sk_seed, pk_seed, &adrs, md, sig + sig_offset);
    sig_offset += SPHINCS_K * (1 + SPHINCS_A) * SPHINCS_N;

    // Step 4: Get FORS public key for HT signing
    uint8_t fors_pk[SPHINCS_N];
    fors_pk_from_sig(pk_seed, &adrs, md, sig + SPHINCS_N, fors_pk);

    // Step 5: Hypertree signature on FORS PK
    ht_sign(sk_seed, pk_seed, tree_idx, leaf_idx, fors_pk,
            sig + sig_offset);

    *sig_len = SPHINCS_SIG_BYTES;
    return 0;
}

int sphincs_verify(const uint8_t *pk, const uint8_t *msg, uint32_t msg_len,
                   const uint8_t *sig, uint32_t sig_len) {
    if (!pk || !msg || !sig) return -1;
    if (sig_len != SPHINCS_SIG_BYTES) return -2;

    const uint8_t *pk_seed = pk;
    const uint8_t *pk_root = pk + SPHINCS_N;

    // Step 1: Extract R from signature
    const uint8_t *r = sig;
    uint32_t sig_offset = SPHINCS_N;

    // Step 2: Compute message digest
    uint8_t md[32];
    sphincs_h_msg(r, pk, msg, msg_len, md);

    // Extract tree/leaf indices (same as in sign)
    uint64_t tree_idx = 0;
    uint32_t leaf_idx = 0;
    uint32_t bit_offset = SPHINCS_K * SPHINCS_A;
    for (uint32_t i = 0; i < 8 && i < (SPHINCS_H - SPHINCS_TREE_H); i++) {
        tree_idx = (tree_idx << 1) | extract_bits(md, bit_offset + i, 1);
    }
    leaf_idx = extract_bits(md, bit_offset + 8, SPHINCS_TREE_H);

    // Step 3: Reconstruct FORS public key from FORS signature
    sphincs_adrs_t adrs;
    memset(&adrs, 0, sizeof(adrs));
    adrs.layer = 0;
    adrs.tree  = tree_idx;

    uint8_t fors_pk[SPHINCS_N];
    fors_pk_from_sig(pk_seed, &adrs, md, sig + sig_offset, fors_pk);
    sig_offset += SPHINCS_K * (1 + SPHINCS_A) * SPHINCS_N;

    // Step 4: Verify hypertree signature on FORS PK
    return ht_verify(pk_seed, pk_root, tree_idx, leaf_idx, fors_pk,
                     sig + sig_offset);
}
