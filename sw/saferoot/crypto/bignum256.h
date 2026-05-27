// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// bignum256.h — 256-bit unsigned integer arithmetic (bare-metal, no libc)
//
// Representation: uint32_t[8], little-endian words (word 0 = LSW).

#ifndef SAFEROOT_BIGNUM256_H_
#define SAFEROOT_BIGNUM256_H_

#include <stdint.h>

// Number of 32-bit words in a 256-bit integer
#define BN256_WORDS 8

// 256-bit unsigned integer type
typedef struct {
    uint32_t w[BN256_WORDS];
} bn256_t;

// ---------------------------------------------------------------------------
// Basic operations
// ---------------------------------------------------------------------------

// Copy src -> dst
void bn256_copy(bn256_t *dst, const bn256_t *src);

// Set dst = 0
void bn256_zero(bn256_t *dst);

// Compare a and b. Returns: -1 if a<b, 0 if a==b, 1 if a>b.
int bn256_cmp(const bn256_t *a, const bn256_t *b);

// Returns 1 if a == 0, else 0.
int bn256_is_zero(const bn256_t *a);

// ---------------------------------------------------------------------------
// Arithmetic
// ---------------------------------------------------------------------------

// dst = a + b. Returns carry (0 or 1).
uint32_t bn256_add(bn256_t *dst, const bn256_t *a, const bn256_t *b);

// dst = a - b. Returns borrow (0 or 1).
uint32_t bn256_sub(bn256_t *dst, const bn256_t *a, const bn256_t *b);

// Multiply a * b producing a 512-bit result in lo (bits 0..255) and hi (bits 256..511).
void bn256_mul(bn256_t *hi, bn256_t *lo, const bn256_t *a, const bn256_t *b);

// dst = a >> 1 (logical right shift by 1 bit)
void bn256_shift_right(bn256_t *dst, const bn256_t *a);

// ---------------------------------------------------------------------------
// Modular operations
// ---------------------------------------------------------------------------

// dst = a mod m  (a is 512 bits: {hi, lo}, m is 256 bits)
void bn256_mod(bn256_t *dst, const bn256_t *hi, const bn256_t *lo,
               const bn256_t *m);

// dst = (a + b) mod m
void bn256_mod_add(bn256_t *dst, const bn256_t *a, const bn256_t *b,
                   const bn256_t *m);

// dst = (a - b) mod m
void bn256_mod_sub(bn256_t *dst, const bn256_t *a, const bn256_t *b,
                   const bn256_t *m);

// dst = (a * b) mod m
void bn256_mod_mul(bn256_t *dst, const bn256_t *a, const bn256_t *b,
                   const bn256_t *m);

// dst = a^(-1) mod m  (extended Euclidean algorithm)
// Returns 0 on success, -1 if inverse does not exist.
int bn256_mod_inv(bn256_t *dst, const bn256_t *a, const bn256_t *m);

// ---------------------------------------------------------------------------
// Serialization (big-endian byte arrays <-> bn256_t)
// ---------------------------------------------------------------------------

// Convert 32-byte big-endian array to bn256_t
void bn256_from_bytes(bn256_t *dst, const uint8_t *bytes);

// Convert bn256_t to 32-byte big-endian array
void bn256_to_bytes(uint8_t *bytes, const bn256_t *src);

#endif  // SAFEROOT_BIGNUM256_H_
