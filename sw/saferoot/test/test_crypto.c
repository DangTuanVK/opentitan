// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// test_crypto.c — Unit tests for AES-256 and HMAC-SHA256 HAL drivers
// Uses NIST FIPS 197 and RFC 4231 test vectors.

#include "test_framework.h"
#include "../hal/aes_drv.h"
#include "../hal/hmac_drv.h"

// ======================================================================
// AES-256 ECB test vector — NIST FIPS 197 Appendix C.3
// Key:       000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
// Plaintext: 00112233445566778899aabbccddeeff
// Expected:  8ea2b7ca516745bfeafc49904b496089
// ======================================================================

// Key as 8 x uint32_t (big-endian word order, each word little-endian bytes)
static const uint32_t aes_key[8] = {
    0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c,
    0x13121110, 0x17161514, 0x1b1a1918, 0x1f1e1d1c,
};

// Plaintext as 4 x uint32_t
static const uint32_t aes_plaintext[4] = {
    0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc,
};

// Expected ciphertext as bytes (for mem compare)
static const uint8_t aes_expected_ct[16] = {
    0x8e, 0xa2, 0xb7, 0xca, 0x51, 0x67, 0x45, 0xbf,
    0xea, 0xfc, 0x49, 0x90, 0x4b, 0x49, 0x60, 0x89,
};

// ======================================================================
// HMAC-SHA256 test vector — RFC 4231 Test Case 2
// Key:   "Jefe" (4 bytes, zero-padded to 32 bytes)
// Data:  "what do ya want for nothing?" (28 bytes)
// HMAC:  5bdcc146bf60754e6a042426089575c7
//        5a003f089d2739839dec58b964ec3843
// ======================================================================

// HMAC key as 8 x uint32_t (zero-padded)
static const uint32_t hmac_key[8] = {
    0x6566654a,  // "Jefe" in little-endian
    0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

// Expected HMAC digest as bytes
static const uint8_t hmac_expected[32] = {
    0x5b, 0xdc, 0xc1, 0x46, 0xbf, 0x60, 0x75, 0x4e,
    0x6a, 0x04, 0x24, 0x26, 0x08, 0x95, 0x75, 0xc7,
    0x5a, 0x00, 0x3f, 0x08, 0x9d, 0x27, 0x39, 0x83,
    0x9d, 0xec, 0x58, 0xb9, 0x64, 0xec, 0x38, 0x43,
};

// HMAC message — must be 4-byte aligned, padded to multiple of 4
static const uint8_t hmac_message[28] __attribute__((aligned(4))) =
    "what do ya want for nothing?";

void test_crypto_suite(void) {
    TEST_SUITE_START("Crypto");

    // ----------------------------------------------------------------
    // AES-256 ECB encrypt
    // ----------------------------------------------------------------
    TEST_CASE("AES-256 ECB encrypt (NIST FIPS 197 C.3)");
    hal_status_t st = aes_init(kAesEncrypt, kAesModeEcb, aes_key, (void *)0);
    uint32_t ct_words[4];
    if (st == kHalOk) {
        st = aes_process_block(aes_plaintext, ct_words);
    }
    if (st == kHalOk) {
        TEST_ASSERT_MEM_EQ(ct_words, aes_expected_ct, 16);
    } else {
        uart_puts("FAIL (AES hw error)\n");
        test_fail_count++;
    }
    aes_clear();

    // ----------------------------------------------------------------
    // AES-256 ECB decrypt (round-trip)
    // ----------------------------------------------------------------
    TEST_CASE("AES-256 ECB decrypt round-trip");
    st = aes_init(kAesDecrypt, kAesModeEcb, aes_key, (void *)0);
    uint32_t pt_words[4];
    if (st == kHalOk) {
        st = aes_process_block(ct_words, pt_words);
    }
    if (st == kHalOk) {
        TEST_ASSERT_MEM_EQ(pt_words, aes_plaintext, 16);
    } else {
        uart_puts("FAIL (AES hw error)\n");
        test_fail_count++;
    }
    aes_clear();

    // ----------------------------------------------------------------
    // HMAC-SHA256 — RFC 4231 Test Case 2
    // ----------------------------------------------------------------
    TEST_CASE("HMAC-SHA256 (RFC 4231 TC2)");
    st = hmac_init(hmac_key);
    if (st == kHalOk) {
        st = hmac_update(hmac_message, 28);
    }
    uint32_t digest[8];
    if (st == kHalOk) {
        st = hmac_final(digest);
    }
    if (st == kHalOk) {
        TEST_ASSERT_MEM_EQ(digest, hmac_expected, 32);
    } else {
        uart_puts("FAIL (HMAC hw error)\n");
        test_fail_count++;
    }
    hmac_wipe(0xDEADDEAD);

    // ----------------------------------------------------------------
    // AES init bad args
    // ----------------------------------------------------------------
    TEST_CASE("AES init rejects NULL key");
    TEST_ASSERT_EQ(aes_init(kAesEncrypt, kAesModeEcb, (void *)0, (void *)0),
                   kHalBadArg);

    TEST_SUITE_END();
}
