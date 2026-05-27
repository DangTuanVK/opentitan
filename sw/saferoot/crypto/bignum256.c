// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// bignum256.c — 256-bit unsigned integer arithmetic (bare-metal, no libc)
//
// Schoolbook multiplication, extended Euclidean modular inverse.
// Optimized for correctness over speed (software fallback for OTBN).

#include "bignum256.h"

// Bare-metal stubs (provided by util.c)
extern void *memcpy(void *dest, const void *src, unsigned int n);
extern void *memset(void *s, int c, unsigned int n);

// ---------------------------------------------------------------------------
// Basic operations
// ---------------------------------------------------------------------------

void bn256_copy(bn256_t *dst, const bn256_t *src) {
    for (int i = 0; i < BN256_WORDS; i++) {
        dst->w[i] = src->w[i];
    }
}

void bn256_zero(bn256_t *dst) {
    for (int i = 0; i < BN256_WORDS; i++) {
        dst->w[i] = 0;
    }
}

int bn256_cmp(const bn256_t *a, const bn256_t *b) {
    for (int i = BN256_WORDS - 1; i >= 0; i--) {
        if (a->w[i] > b->w[i]) return 1;
        if (a->w[i] < b->w[i]) return -1;
    }
    return 0;
}

int bn256_is_zero(const bn256_t *a) {
    uint32_t acc = 0;
    for (int i = 0; i < BN256_WORDS; i++) {
        acc |= a->w[i];
    }
    return (acc == 0) ? 1 : 0;
}

// ---------------------------------------------------------------------------
// Addition / Subtraction
// ---------------------------------------------------------------------------

uint32_t bn256_add(bn256_t *dst, const bn256_t *a, const bn256_t *b) {
    uint64_t carry = 0;
    for (int i = 0; i < BN256_WORDS; i++) {
        carry += (uint64_t)a->w[i] + (uint64_t)b->w[i];
        dst->w[i] = (uint32_t)carry;
        carry >>= 32;
    }
    return (uint32_t)carry;
}

uint32_t bn256_sub(bn256_t *dst, const bn256_t *a, const bn256_t *b) {
    uint64_t borrow = 0;
    for (int i = 0; i < BN256_WORDS; i++) {
        uint64_t diff = (uint64_t)a->w[i] - (uint64_t)b->w[i] - borrow;
        dst->w[i] = (uint32_t)diff;
        borrow = (diff >> 63) & 1;  // borrow if result was negative
    }
    return (uint32_t)borrow;
}

// ---------------------------------------------------------------------------
// Shift
// ---------------------------------------------------------------------------

void bn256_shift_right(bn256_t *dst, const bn256_t *a) {
    for (int i = 0; i < BN256_WORDS - 1; i++) {
        dst->w[i] = (a->w[i] >> 1) | (a->w[i + 1] << 31);
    }
    dst->w[BN256_WORDS - 1] = a->w[BN256_WORDS - 1] >> 1;
}

// ---------------------------------------------------------------------------
// Multiplication: a * b -> (hi, lo) 512-bit result
// Schoolbook O(n^2) — correct and simple
// ---------------------------------------------------------------------------

void bn256_mul(bn256_t *hi, bn256_t *lo, const bn256_t *a, const bn256_t *b) {
    uint32_t result[BN256_WORDS * 2];
    for (int i = 0; i < BN256_WORDS * 2; i++) {
        result[i] = 0;
    }

    for (int i = 0; i < BN256_WORDS; i++) {
        uint64_t carry = 0;
        for (int j = 0; j < BN256_WORDS; j++) {
            uint64_t prod = (uint64_t)a->w[i] * (uint64_t)b->w[j]
                          + (uint64_t)result[i + j] + carry;
            result[i + j] = (uint32_t)prod;
            carry = prod >> 32;
        }
        result[i + BN256_WORDS] = (uint32_t)carry;
    }

    for (int i = 0; i < BN256_WORDS; i++) {
        lo->w[i] = result[i];
        hi->w[i] = result[i + BN256_WORDS];
    }
}

// ---------------------------------------------------------------------------
// Modular reduction: dst = {hi, lo} mod m
// Uses repeated subtraction with shifting (binary long division).
// ---------------------------------------------------------------------------

void bn256_mod(bn256_t *dst, const bn256_t *hi, const bn256_t *lo,
               const bn256_t *m) {
    // Work with a 512-bit dividend represented as 16 words
    uint32_t dividend[16];
    for (int i = 0; i < 8; i++) {
        dividend[i] = lo->w[i];
        dividend[i + 8] = hi->w[i];
    }

    // Find the highest set bit of the dividend
    int dividend_bits = 0;
    for (int i = 15; i >= 0; i--) {
        if (dividend[i] != 0) {
            uint32_t v = dividend[i];
            int bit = 0;
            while (v >>= 1) bit++;
            dividend_bits = i * 32 + bit + 1;
            break;
        }
    }

    // Find highest set bit of modulus
    int mod_bits = 0;
    for (int i = BN256_WORDS - 1; i >= 0; i--) {
        if (m->w[i] != 0) {
            uint32_t v = m->w[i];
            int bit = 0;
            while (v >>= 1) bit++;
            mod_bits = i * 32 + bit + 1;
            break;
        }
    }

    if (mod_bits == 0) {
        // modulus is zero — undefined; just zero out
        bn256_zero(dst);
        return;
    }

    // Shift modulus left so its MSB aligns with dividend's MSB
    int shift = dividend_bits - mod_bits;
    if (shift < 0) {
        // dividend < modulus
        for (int i = 0; i < BN256_WORDS; i++) {
            dst->w[i] = dividend[i];
        }
        return;
    }

    // Shifted modulus in 16-word form
    uint32_t shifted_m[16];

    // Perform long division
    for (int s = shift; s >= 0; s--) {
        // Compute shifted_m = m << s (in 512-bit space)
        for (int i = 0; i < 16; i++) shifted_m[i] = 0;
        int word_shift = s / 32;
        int bit_shift = s % 32;
        for (int i = 0; i < BN256_WORDS; i++) {
            int dst_idx = i + word_shift;
            if (dst_idx < 16) {
                shifted_m[dst_idx] |= m->w[i] << bit_shift;
            }
            if (bit_shift > 0 && dst_idx + 1 < 16) {
                shifted_m[dst_idx + 1] |= m->w[i] >> (32 - bit_shift);
            }
        }

        // If dividend >= shifted_m, subtract
        int ge = 1;
        for (int i = 15; i >= 0; i--) {
            if (dividend[i] > shifted_m[i]) break;
            if (dividend[i] < shifted_m[i]) { ge = 0; break; }
        }

        if (ge) {
            uint64_t borrow = 0;
            for (int i = 0; i < 16; i++) {
                uint64_t diff = (uint64_t)dividend[i]
                              - (uint64_t)shifted_m[i] - borrow;
                dividend[i] = (uint32_t)diff;
                borrow = (diff >> 63) & 1;
            }
        }
    }

    for (int i = 0; i < BN256_WORDS; i++) {
        dst->w[i] = dividend[i];
    }
}

// ---------------------------------------------------------------------------
// Modular arithmetic helpers
// ---------------------------------------------------------------------------

void bn256_mod_add(bn256_t *dst, const bn256_t *a, const bn256_t *b,
                   const bn256_t *m) {
    uint32_t carry = bn256_add(dst, a, b);
    if (carry || bn256_cmp(dst, m) >= 0) {
        bn256_sub(dst, dst, m);
    }
}

void bn256_mod_sub(bn256_t *dst, const bn256_t *a, const bn256_t *b,
                   const bn256_t *m) {
    uint32_t borrow = bn256_sub(dst, a, b);
    if (borrow) {
        bn256_add(dst, dst, m);
    }
}

void bn256_mod_mul(bn256_t *dst, const bn256_t *a, const bn256_t *b,
                   const bn256_t *m) {
    bn256_t hi, lo;
    bn256_mul(&hi, &lo, a, b);
    bn256_mod(dst, &hi, &lo, m);
}

// ---------------------------------------------------------------------------
// Modular inverse: dst = a^(-1) mod m
// Uses binary extended GCD (constant-time-ish version).
// Returns 0 on success, -1 if gcd(a, m) != 1.
// ---------------------------------------------------------------------------

int bn256_mod_inv(bn256_t *dst, const bn256_t *a, const bn256_t *m) {
    if (bn256_is_zero(a)) {
        bn256_zero(dst);
        return -1;
    }

    bn256_t u, v, x1, x2;
    bn256_copy(&u, a);
    bn256_copy(&v, m);

    // x1 = 1, x2 = 0
    bn256_zero(&x1);
    x1.w[0] = 1;
    bn256_zero(&x2);

    while (!bn256_is_zero(&u) && !bn256_is_zero(&v)) {
        // While u is even
        while ((u.w[0] & 1) == 0) {
            bn256_shift_right(&u, &u);
            if (x1.w[0] & 1) {
                // x1 = (x1 + m) / 2
                uint32_t carry = bn256_add(&x1, &x1, m);
                bn256_shift_right(&x1, &x1);
                if (carry) {
                    x1.w[BN256_WORDS - 1] |= 0x80000000U;
                }
            } else {
                bn256_shift_right(&x1, &x1);
            }
        }

        // While v is even
        while ((v.w[0] & 1) == 0) {
            bn256_shift_right(&v, &v);
            if (x2.w[0] & 1) {
                uint32_t carry = bn256_add(&x2, &x2, m);
                bn256_shift_right(&x2, &x2);
                if (carry) {
                    x2.w[BN256_WORDS - 1] |= 0x80000000U;
                }
            } else {
                bn256_shift_right(&x2, &x2);
            }
        }

        if (bn256_cmp(&u, &v) >= 0) {
            bn256_sub(&u, &u, &v);
            bn256_mod_sub(&x1, &x1, &x2, m);
        } else {
            bn256_sub(&v, &v, &u);
            bn256_mod_sub(&x2, &x2, &x1, m);
        }
    }

    // Check that gcd == 1
    bn256_t one;
    bn256_zero(&one);
    one.w[0] = 1;

    if (bn256_is_zero(&u)) {
        // Result is in x2 if v == 1
        if (bn256_cmp(&v, &one) != 0) return -1;
        bn256_copy(dst, &x2);
    } else {
        // Result is in x1 if u == 1
        if (bn256_cmp(&u, &one) != 0) return -1;
        bn256_copy(dst, &x1);
    }

    // Ensure result is in [0, m)
    if (bn256_cmp(dst, m) >= 0) {
        bn256_sub(dst, dst, m);
    }

    return 0;
}

// ---------------------------------------------------------------------------
// Serialization: big-endian byte array <-> bn256_t (little-endian words)
// ---------------------------------------------------------------------------

void bn256_from_bytes(bn256_t *dst, const uint8_t *bytes) {
    for (int i = 0; i < BN256_WORDS; i++) {
        int byte_idx = (BN256_WORDS - 1 - i) * 4;
        dst->w[i] = ((uint32_t)bytes[byte_idx]     << 24) |
                    ((uint32_t)bytes[byte_idx + 1] << 16) |
                    ((uint32_t)bytes[byte_idx + 2] <<  8) |
                    ((uint32_t)bytes[byte_idx + 3]);
    }
}

void bn256_to_bytes(uint8_t *bytes, const bn256_t *src) {
    for (int i = 0; i < BN256_WORDS; i++) {
        int byte_idx = (BN256_WORDS - 1 - i) * 4;
        bytes[byte_idx]     = (uint8_t)(src->w[i] >> 24);
        bytes[byte_idx + 1] = (uint8_t)(src->w[i] >> 16);
        bytes[byte_idx + 2] = (uint8_t)(src->w[i] >>  8);
        bytes[byte_idx + 3] = (uint8_t)(src->w[i]);
    }
}
