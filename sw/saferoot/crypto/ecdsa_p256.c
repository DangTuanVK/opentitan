// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// ecdsa_p256.c — ECDSA P-256 software implementation (bare-metal rv32imc)
//
// Software fallback based on micro-ecc (public domain) approach.
// No malloc, no libc. Uses CSRNG hardware for random nonce generation.
//
// Optimized for correctness first, performance second.
// OTBN coprocessor will replace this implementation when ready.
//
// Curve: NIST P-256 (secp256r1 / prime256v1)
//   p = 2^256 - 2^224 + 2^192 + 2^96 - 1
//   a = p - 3
//   n = FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551
//   G = (Gx, Gy) as defined below

#include "ecdsa_p256.h"
#include "bignum256.h"

// Bare-metal stubs (provided by util.c)
extern void *memcpy(void *dest, const void *src, unsigned int n);
extern void *memset(void *s, int c, unsigned int n);

// ---------------------------------------------------------------------------
// CSRNG hardware registers (OpenTitan CSRNG IP at 0x40030000)
// ---------------------------------------------------------------------------
#define CSRNG_BASE          0x40030000U
#define CSRNG_CMD_REQ       (CSRNG_BASE + 0x14U)
#define CSRNG_GENBITS_VLD   (CSRNG_BASE + 0x1CU)
#define CSRNG_GENBITS       (CSRNG_BASE + 0x20U)

#define CSRNG_CMD_GENERATE  0x03U

#define REG32_READ(addr)    (*(volatile uint32_t *)(addr))
#define REG32_WRITE(addr, val) \
    do { (*(volatile uint32_t *)(addr)) = (val); } while (0)

// ---------------------------------------------------------------------------
// P-256 curve parameters (little-endian uint32_t[8])
// ---------------------------------------------------------------------------

// p = FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF
static const bn256_t P256_P = {{
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000,
    0x00000000, 0x00000000, 0x00000001, 0xFFFFFFFF
}};

// n = FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551
static const bn256_t P256_N = {{
    0xFC632551, 0xF3B9CAC2, 0xA7179E84, 0xBCE6FAAD,
    0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF
}};

// b = 5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B
static const bn256_t P256_B = {{
    0x27D2604B, 0x3BCE3C3E, 0xCC53B0F6, 0x651D06B0,
    0x769886BC, 0xB3EBBD55, 0xAA3A93E7, 0x5AC635D8
}};

// Gx = 6B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C296
static const bn256_t P256_GX = {{
    0xD898C296, 0xF4A13945, 0x2DEB33A0, 0x77037D81,
    0x63A440F2, 0xF8BCE6E5, 0xE12C4247, 0x6B17D1F2
}};

// Gy = 4FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5
static const bn256_t P256_GY = {{
    0x37BF51F5, 0xCBB64068, 0x6B315ECE, 0x2BCE3357,
    0x7C0F9E16, 0x8EE7EB4A, 0xFE1A7F9B, 0x4FE342E2
}};

// ---------------------------------------------------------------------------
// Point type (affine coordinates)
// ---------------------------------------------------------------------------
typedef struct {
    bn256_t x;
    bn256_t y;
} point_t;

// Point at infinity marker: both coordinates zero
static int point_is_infinity(const point_t *p) {
    return bn256_is_zero(&p->x) && bn256_is_zero(&p->y);
}

static void point_set_infinity(point_t *p) {
    bn256_zero(&p->x);
    bn256_zero(&p->y);
}

// ---------------------------------------------------------------------------
// Modular arithmetic wrappers for field Fp
// ---------------------------------------------------------------------------

static void fp_add(bn256_t *dst, const bn256_t *a, const bn256_t *b) {
    bn256_mod_add(dst, a, b, &P256_P);
}

static void fp_sub(bn256_t *dst, const bn256_t *a, const bn256_t *b) {
    bn256_mod_sub(dst, a, b, &P256_P);
}

static void fp_mul(bn256_t *dst, const bn256_t *a, const bn256_t *b) {
    bn256_mod_mul(dst, a, b, &P256_P);
}

static int fp_inv(bn256_t *dst, const bn256_t *a) {
    return bn256_mod_inv(dst, a, &P256_P);
}

// ---------------------------------------------------------------------------
// Modular arithmetic wrappers for order Fn (mod n)
// ---------------------------------------------------------------------------

static void fn_add(bn256_t *dst, const bn256_t *a, const bn256_t *b) {
    bn256_mod_add(dst, a, b, &P256_N);
}

static void fn_mul(bn256_t *dst, const bn256_t *a, const bn256_t *b) {
    bn256_mod_mul(dst, a, b, &P256_N);
}

static int fn_inv(bn256_t *dst, const bn256_t *a) {
    return bn256_mod_inv(dst, a, &P256_N);
}

// ---------------------------------------------------------------------------
// Point doubling: R = 2*P (affine coordinates)
// ---------------------------------------------------------------------------
static void point_double(point_t *r, const point_t *p) {
    if (point_is_infinity(p) || bn256_is_zero(&p->y)) {
        point_set_infinity(r);
        return;
    }

    bn256_t lambda, tmp, tmp2;

    // lambda = (3*x^2 + a) / (2*y)
    // For P-256, a = p - 3, so 3*x^2 + a = 3*(x^2 - 1) + p = 3*x^2 - 3 mod p
    fp_mul(&tmp, &p->x, &p->x);       // tmp = x^2

    bn256_t three;
    bn256_zero(&three);
    three.w[0] = 3;

    fp_mul(&tmp, &tmp, &three);        // tmp = 3*x^2
    fp_sub(&tmp, &tmp, &three);        // tmp = 3*x^2 - 3 (since a = -3)

    bn256_t two_y;
    bn256_mod_add(&two_y, &p->y, &p->y, &P256_P);  // two_y = 2*y

    bn256_t inv_2y;
    fp_inv(&inv_2y, &two_y);           // inv_2y = (2y)^(-1)
    fp_mul(&lambda, &tmp, &inv_2y);    // lambda = (3x^2 - 3) / (2y)

    // xr = lambda^2 - 2*x
    fp_mul(&tmp, &lambda, &lambda);    // tmp = lambda^2
    fp_sub(&tmp, &tmp, &p->x);        // tmp = lambda^2 - x
    fp_sub(&tmp, &tmp, &p->x);        // tmp = lambda^2 - 2x  -> xr

    // yr = lambda * (x - xr) - y
    fp_sub(&tmp2, &p->x, &tmp);       // tmp2 = x - xr
    fp_mul(&tmp2, &lambda, &tmp2);     // tmp2 = lambda*(x - xr)
    fp_sub(&tmp2, &tmp2, &p->y);      // tmp2 = lambda*(x - xr) - y -> yr

    bn256_copy(&r->x, &tmp);
    bn256_copy(&r->y, &tmp2);
}

// ---------------------------------------------------------------------------
// Point addition: R = P + Q (affine coordinates)
// ---------------------------------------------------------------------------
static void point_add(point_t *r, const point_t *p, const point_t *q) {
    if (point_is_infinity(p)) {
        bn256_copy(&r->x, &q->x);
        bn256_copy(&r->y, &q->y);
        return;
    }
    if (point_is_infinity(q)) {
        bn256_copy(&r->x, &p->x);
        bn256_copy(&r->y, &p->y);
        return;
    }

    // Check if P == Q -> point doubling
    if (bn256_cmp(&p->x, &q->x) == 0) {
        if (bn256_cmp(&p->y, &q->y) == 0) {
            point_double(r, p);
            return;
        }
        // P == -Q -> infinity
        point_set_infinity(r);
        return;
    }

    bn256_t lambda, dx, dy, tmp, tmp2;

    // lambda = (y2 - y1) / (x2 - x1)
    fp_sub(&dy, &q->y, &p->y);
    fp_sub(&dx, &q->x, &p->x);

    bn256_t inv_dx;
    fp_inv(&inv_dx, &dx);
    fp_mul(&lambda, &dy, &inv_dx);

    // xr = lambda^2 - x1 - x2
    fp_mul(&tmp, &lambda, &lambda);
    fp_sub(&tmp, &tmp, &p->x);
    fp_sub(&tmp, &tmp, &q->x);

    // yr = lambda * (x1 - xr) - y1
    fp_sub(&tmp2, &p->x, &tmp);
    fp_mul(&tmp2, &lambda, &tmp2);
    fp_sub(&tmp2, &tmp2, &p->y);

    bn256_copy(&r->x, &tmp);
    bn256_copy(&r->y, &tmp2);
}

// ---------------------------------------------------------------------------
// Scalar multiplication: R = k * P
// Double-and-add, processing from MSB to LSB for constant-time-ish behavior.
// ---------------------------------------------------------------------------
static void scalar_multiply(point_t *r, const bn256_t *k, const point_t *p) {
    point_t result;
    point_set_infinity(&result);

    point_t current;
    bn256_copy(&current.x, &p->x);
    bn256_copy(&current.y, &p->y);

    // Process bits from LSB to MSB
    for (int i = 0; i < 256; i++) {
        int word = i / 32;
        int bit = i % 32;

        if (k->w[word] & (1U << bit)) {
            point_t tmp;
            point_add(&tmp, &result, &current);
            bn256_copy(&result.x, &tmp.x);
            bn256_copy(&result.y, &tmp.y);
        }

        point_t doubled;
        point_double(&doubled, &current);
        bn256_copy(&current.x, &doubled.x);
        bn256_copy(&current.y, &doubled.y);
    }

    bn256_copy(&r->x, &result.x);
    bn256_copy(&r->y, &result.y);
}

// ---------------------------------------------------------------------------
// CSRNG: generate random bytes from hardware
// ---------------------------------------------------------------------------
static int csrng_random_bytes(uint8_t *buf, uint32_t len) {
    uint32_t num_blocks = (len + 15) / 16;
    uint32_t cmd_word = (num_blocks << 12) | CSRNG_CMD_GENERATE;
    REG32_WRITE(CSRNG_CMD_REQ, cmd_word);

    uint32_t offset = 0;
    for (uint32_t blk = 0; blk < num_blocks; blk++) {
        // Wait for genbits_vld
        uint32_t timeout = 1000000;
        while ((REG32_READ(CSRNG_GENBITS_VLD) & 0x01U) == 0) {
            if (--timeout == 0) return -1;
        }
        // Read 4 words (128 bits)
        for (uint32_t w = 0; w < 4; w++) {
            uint32_t rnd = REG32_READ(CSRNG_GENBITS);
            for (uint32_t b = 0; b < 4 && offset < len; b++, offset++) {
                buf[offset] = (uint8_t)(rnd >> (b * 8));
            }
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Generate a random scalar in [1, n-1]
// ---------------------------------------------------------------------------
static int generate_random_scalar(bn256_t *k) {
    bn256_t one;
    bn256_zero(&one);
    one.w[0] = 1;

    // Try up to 64 times to get a valid random scalar
    for (int attempt = 0; attempt < 64; attempt++) {
        uint8_t bytes[32];
        if (csrng_random_bytes(bytes, 32) != 0) {
            return -1;
        }
        bn256_from_bytes(k, bytes);

        // Check: 0 < k < n
        if (bn256_is_zero(k)) continue;
        if (bn256_cmp(k, &P256_N) >= 0) continue;

        // Clear temporary
        memset(bytes, 0, 32);
        return 0;
    }
    return -1;  // Failed to generate valid scalar
}

// ---------------------------------------------------------------------------
// Validate that a point is on the curve: y^2 = x^3 + ax + b mod p
// ---------------------------------------------------------------------------
static int point_is_on_curve(const point_t *pt) {
    if (point_is_infinity(pt)) return 0;  // infinity is not valid for pubkey

    bn256_t lhs, rhs, tmp;

    // lhs = y^2 mod p
    fp_mul(&lhs, &pt->y, &pt->y);

    // rhs = x^3 + a*x + b = x^3 - 3*x + b mod p (since a = -3)
    fp_mul(&tmp, &pt->x, &pt->x);       // x^2
    fp_mul(&rhs, &tmp, &pt->x);         // x^3

    bn256_t three_x;
    bn256_t three;
    bn256_zero(&three);
    three.w[0] = 3;
    fp_mul(&three_x, &three, &pt->x);   // 3*x

    fp_sub(&rhs, &rhs, &three_x);       // x^3 - 3*x
    fp_add(&rhs, &rhs, &P256_B);        // x^3 - 3*x + b

    return (bn256_cmp(&lhs, &rhs) == 0) ? 1 : 0;
}

// ---------------------------------------------------------------------------
// ECDSA Sign
// ---------------------------------------------------------------------------
int ecdsa_p256_sign(const uint8_t *private_key,
                    const uint8_t *hash,
                    uint8_t *signature) {
    bn256_t d, z, k, r_bn, s_bn;
    point_t R;

    // Parse private key and hash
    bn256_from_bytes(&d, private_key);
    bn256_from_bytes(&z, hash);

    // Validate private key: 0 < d < n
    if (bn256_is_zero(&d) || bn256_cmp(&d, &P256_N) >= 0) {
        return -1;
    }

    // Truncate hash to n bits if necessary (SHA-256 is exactly 256 bits, same as n)
    // If z >= n, reduce: z = z mod n
    if (bn256_cmp(&z, &P256_N) >= 0) {
        bn256_t zero_hi;
        bn256_zero(&zero_hi);
        bn256_mod(&z, &zero_hi, &z, &P256_N);
    }

    // Try signing with random k
    for (int attempt = 0; attempt < 64; attempt++) {
        // Generate random nonce k in [1, n-1]
        if (generate_random_scalar(&k) != 0) {
            return -2;  // RNG failure
        }

        // R = k * G
        point_t G;
        bn256_copy(&G.x, &P256_GX);
        bn256_copy(&G.y, &P256_GY);
        scalar_multiply(&R, &k, &G);

        if (point_is_infinity(&R)) continue;

        // r = R.x mod n
        if (bn256_cmp(&R.x, &P256_N) >= 0) {
            bn256_sub(&r_bn, &R.x, &P256_N);
        } else {
            bn256_copy(&r_bn, &R.x);
        }
        if (bn256_is_zero(&r_bn)) continue;

        // s = k^(-1) * (z + r * d) mod n
        bn256_t rd, z_plus_rd, k_inv;
        fn_mul(&rd, &r_bn, &d);           // r * d mod n
        fn_add(&z_plus_rd, &z, &rd);      // z + r*d mod n

        if (fn_inv(&k_inv, &k) != 0) continue;  // k not invertible (shouldn't happen)

        fn_mul(&s_bn, &k_inv, &z_plus_rd);  // s = k^(-1) * (z + r*d) mod n

        if (bn256_is_zero(&s_bn)) continue;

        // Encode signature: r || s (big-endian, 32 bytes each)
        bn256_to_bytes(signature, &r_bn);
        bn256_to_bytes(signature + 32, &s_bn);

        // Clear sensitive data
        bn256_zero(&k);
        bn256_zero(&k_inv);
        bn256_zero(&d);

        return 0;  // Success
    }

    return -3;  // Failed after all attempts
}

// ---------------------------------------------------------------------------
// ECDSA Verify
// ---------------------------------------------------------------------------
int ecdsa_p256_verify(const uint8_t *public_key,
                      const uint8_t *hash,
                      const uint8_t *signature) {
    bn256_t r, s, z;
    point_t Q;

    // Parse inputs
    bn256_from_bytes(&r, signature);
    bn256_from_bytes(&s, signature + 32);
    bn256_from_bytes(&z, hash);
    bn256_from_bytes(&Q.x, public_key);
    bn256_from_bytes(&Q.y, public_key + 32);

    // Check r, s in [1, n-1]
    if (bn256_is_zero(&r) || bn256_cmp(&r, &P256_N) >= 0) return -1;
    if (bn256_is_zero(&s) || bn256_cmp(&s, &P256_N) >= 0) return -1;

    // Validate public key is on curve
    if (!point_is_on_curve(&Q)) return -2;

    // Truncate hash if needed
    if (bn256_cmp(&z, &P256_N) >= 0) {
        bn256_t zero_hi;
        bn256_zero(&zero_hi);
        bn256_mod(&z, &zero_hi, &z, &P256_N);
    }

    // s_inv = s^(-1) mod n
    bn256_t s_inv;
    if (fn_inv(&s_inv, &s) != 0) return -3;

    // u1 = z * s^(-1) mod n
    bn256_t u1;
    fn_mul(&u1, &z, &s_inv);

    // u2 = r * s^(-1) mod n
    bn256_t u2;
    fn_mul(&u2, &r, &s_inv);

    // P = u1*G + u2*Q
    point_t G;
    bn256_copy(&G.x, &P256_GX);
    bn256_copy(&G.y, &P256_GY);

    point_t p1, p2, P_result;
    scalar_multiply(&p1, &u1, &G);
    scalar_multiply(&p2, &u2, &Q);
    point_add(&P_result, &p1, &p2);

    if (point_is_infinity(&P_result)) return -4;

    // Check: P.x mod n == r
    bn256_t v;
    if (bn256_cmp(&P_result.x, &P256_N) >= 0) {
        bn256_sub(&v, &P_result.x, &P256_N);
    } else {
        bn256_copy(&v, &P_result.x);
    }

    // Constant-time comparison
    uint32_t diff = 0;
    for (int i = 0; i < BN256_WORDS; i++) {
        diff |= v.w[i] ^ r.w[i];
    }

    return (diff == 0) ? 0 : -5;
}

// ---------------------------------------------------------------------------
// Key generation
// ---------------------------------------------------------------------------
int ecdsa_p256_keygen(uint8_t *private_key, uint8_t *public_key) {
    bn256_t d;

    // Generate random private key in [1, n-1]
    if (generate_random_scalar(&d) != 0) {
        return -1;
    }

    // Encode private key
    bn256_to_bytes(private_key, &d);

    // Compute public key = d * G
    point_t G, Q;
    bn256_copy(&G.x, &P256_GX);
    bn256_copy(&G.y, &P256_GY);
    scalar_multiply(&Q, &d, &G);

    // Encode public key: x || y
    bn256_to_bytes(public_key, &Q.x);
    bn256_to_bytes(public_key + 32, &Q.y);

    // Clear private key from stack
    bn256_zero(&d);

    return 0;
}

// ---------------------------------------------------------------------------
// Compute public key from private key
// ---------------------------------------------------------------------------
int ecdsa_p256_compute_public(const uint8_t *private_key,
                              uint8_t *public_key) {
    bn256_t d;
    bn256_from_bytes(&d, private_key);

    // Validate: 0 < d < n
    if (bn256_is_zero(&d) || bn256_cmp(&d, &P256_N) >= 0) {
        return -1;
    }

    // Q = d * G
    point_t G, Q;
    bn256_copy(&G.x, &P256_GX);
    bn256_copy(&G.y, &P256_GY);
    scalar_multiply(&Q, &d, &G);

    if (point_is_infinity(&Q)) {
        bn256_zero(&d);
        return -2;
    }

    // Validate result is on curve
    if (!point_is_on_curve(&Q)) {
        bn256_zero(&d);
        return -3;
    }

    bn256_to_bytes(public_key, &Q.x);
    bn256_to_bytes(public_key + 32, &Q.y);

    bn256_zero(&d);
    return 0;
}
