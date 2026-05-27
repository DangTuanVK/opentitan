// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// test_se_crypto.c -- Integration test: SE init -> keygen -> encrypt -> decrypt
// Exercises the Secure Element cryptographic pipeline end-to-end.

#include "../test/test_framework.h"
#include "../hal/hal.h"
#include "../se/se.h"
#include "../se/se_crypto.h"
#include "../se/se_keystore.h"

// ---------------------------------------------------------------------------
// Known plaintext for AES round-trip (32 bytes = 2 AES blocks)
// ---------------------------------------------------------------------------
static const uint8_t known_plaintext[32] __attribute__((aligned(4))) = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
    0x10, 0x21, 0x32, 0x43, 0x54, 0x65, 0x76, 0x87,
    0x98, 0xA9, 0xBA, 0xCB, 0xDC, 0xED, 0xFE, 0x0F,
};

#define AES_TEST_SLOT  0
#define AES_TEST_LEN   32U  // must be multiple of 16

void test_se_crypto_suite(void) {
    TEST_SUITE_START("SE Crypto");

    // -- Init SE subsystem --
    se_init();

    // -- Test 1: Generate AES-256 key in slot 0 --
    TEST_CASE("keygen AES-256 in slot 0");
    uint16_t sw = keystore_generate(AES_TEST_SLOT, kKeyTypeAes256);
    TEST_ASSERT_EQ(sw, SW_OK);

    // -- Test 2: Verify key slot is now active --
    TEST_CASE("slot 0 is active after keygen");
    se_key_t key_info;
    sw = keystore_get(AES_TEST_SLOT, &key_info);
    TEST_ASSERT(sw == SW_OK &&
                (key_info.flags & SE_KEY_FLAG_ACTIVE) != 0);

    // -- Test 3: Encrypt known plaintext --
    TEST_CASE("AES-256 encrypt succeeds");
    uint8_t ciphertext[AES_TEST_LEN];
    sw = se_aes_encrypt(AES_TEST_SLOT, known_plaintext,
                        ciphertext, AES_TEST_LEN);
    TEST_ASSERT_EQ(sw, SW_OK);

    // -- Test 4: Ciphertext differs from plaintext --
    TEST_CASE("ciphertext differs from plaintext");
    bool differs = false;
    for (uint32_t i = 0; i < AES_TEST_LEN; i++) {
        if (ciphertext[i] != known_plaintext[i]) {
            differs = true;
            break;
        }
    }
    TEST_ASSERT(differs);

    // -- Test 5: Decrypt ciphertext --
    TEST_CASE("AES-256 decrypt succeeds");
    uint8_t decrypted[AES_TEST_LEN];
    sw = se_aes_decrypt(AES_TEST_SLOT, ciphertext,
                        decrypted, AES_TEST_LEN);
    TEST_ASSERT_EQ(sw, SW_OK);

    // -- Test 6: Decrypted matches original plaintext --
    TEST_CASE("decrypt matches original plaintext");
    TEST_ASSERT_MEM_EQ(decrypted, known_plaintext, AES_TEST_LEN);

    // -- Test 7: get_random returns non-zero data --
    TEST_CASE("get_random returns non-zero output");
    uint8_t rand_buf[16];
    sw = se_get_random(rand_buf, 16);
    bool non_zero = false;
    if (sw == SW_OK) {
        for (uint32_t i = 0; i < 16; i++) {
            if (rand_buf[i] != 0) { non_zero = true; break; }
        }
    }
    TEST_ASSERT(sw == SW_OK && non_zero);

    // -- Test 8: Two get_random calls produce different output --
    TEST_CASE("two get_random calls differ");
    uint8_t rand_buf2[16];
    sw = se_get_random(rand_buf2, 16);
    bool different = false;
    if (sw == SW_OK) {
        for (uint32_t i = 0; i < 16; i++) {
            if (rand_buf[i] != rand_buf2[i]) {
                different = true;
                break;
            }
        }
    }
    TEST_ASSERT(sw == SW_OK && different);

    // -- Test 9: keygen on invalid slot fails --
    TEST_CASE("keygen on invalid slot returns error");
    sw = keystore_generate(SE_MAX_KEY_SLOTS, kKeyTypeAes256);
    TEST_ASSERT_EQ(sw, SW_WRONG_DATA);

    // -- Cleanup: delete test key --
    keystore_delete(AES_TEST_SLOT);

    TEST_SUITE_END();
}
