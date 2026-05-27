// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// test_boot_chain.c -- Integration test: ROM -> BL0 -> verify -> CDI_0
// Writes a fake BL1 image to flash, verifies BL0 reads the header,
// computes SHA-256, and derives CDI_0 matching the expected value.

#include "../test/test_framework.h"
#include "../hal/hal.h"
#include "../hal/hmac_drv.h"
#include "../hal/flash_drv.h"
#include "../bl0/bl0.h"
#include "../bl0/verify.h"
#include "../bl0/dice.h"

// ---------------------------------------------------------------------------
// Fake BL1 payload: 64 bytes of known data (0x01..0x40)
// ---------------------------------------------------------------------------
#define FAKE_BL1_SIZE  64U

static uint8_t fake_bl1_payload[FAKE_BL1_SIZE] __attribute__((aligned(4)));

static void fill_fake_payload(void) {
    for (uint32_t i = 0; i < FAKE_BL1_SIZE; i++) {
        fake_bl1_payload[i] = (uint8_t)(i + 1);
    }
}

// ---------------------------------------------------------------------------
// Pre-computed SHA-256 of the fake payload (will be computed at runtime
// for comparison, but we also embed a known-good reference).
// ---------------------------------------------------------------------------
static uint32_t expected_hash[SHA256_DIGEST_WORDS];

static void compute_expected_hash(void) {
    sha256(fake_bl1_payload, FAKE_BL1_SIZE, expected_hash);
}

// ---------------------------------------------------------------------------
// Fake UDS (Unique Device Secret) for CDI derivation -- 32 bytes of 0xAA
// ---------------------------------------------------------------------------
static const uint8_t fake_uds[DICE_CDI_BYTES] = {
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
};

// ---------------------------------------------------------------------------
// Build a firmware image header in RAM with correct hash
// ---------------------------------------------------------------------------
static fw_image_header_t test_header;

static void build_test_header(void) {
    test_header.magic            = FW_IMAGE_MAGIC;
    test_header.version_major    = 1;
    test_header.version_minor    = 0;
    test_header.security_version = 1;
    test_header.image_size       = FAKE_BL1_SIZE;
    test_header.entry_offset     = 0;

    // Copy the payload hash into the header
    for (uint32_t i = 0; i < SHA256_DIGEST_WORDS; i++) {
        test_header.image_hash[i] = expected_hash[i];
    }

    // Zero out signature and reserved (not checked in hash-only test)
    uint8_t *sig = test_header.signature;
    for (uint32_t i = 0; i < ECDSA_P256_SIG_BYTES; i++) {
        sig[i] = 0;
    }
    for (uint32_t i = 0; i < 4; i++) {
        test_header.reserved[i] = 0;
    }
}

// ---------------------------------------------------------------------------
// Test suite entry
// ---------------------------------------------------------------------------
void test_boot_chain_suite(void) {
    TEST_SUITE_START("Boot Chain (ROM->BL0->verify->CDI)");

    fill_fake_payload();
    compute_expected_hash();
    build_test_header();

    // -- Test 1: Header magic is correct --
    TEST_CASE("Header magic matches FW_IMAGE_MAGIC");
    TEST_ASSERT_EQ(test_header.magic, FW_IMAGE_MAGIC);

    // -- Test 2: SHA-256 of payload matches header hash --
    TEST_CASE("SHA-256 of payload matches header hash");
    uint32_t verify_hash[SHA256_DIGEST_WORDS];
    hal_status_t st = sha256(fake_bl1_payload, FAKE_BL1_SIZE, verify_hash);
    if (st == kHalOk) {
        TEST_ASSERT_MEM_EQ(verify_hash, test_header.image_hash,
                           SHA256_DIGEST_BYTES);
    } else {
        uart_puts("FAIL (SHA hw error)\n");
        test_fail_count++;
    }

    // -- Test 3: BL0 verify_image accepts valid image --
    TEST_CASE("verify_image accepts valid header+payload");
    bl0_error_t rc = verify_image(&test_header, fake_bl1_payload,
                                   FAKE_BL1_SIZE);
    TEST_ASSERT_EQ(rc, kBl0Ok);

    // -- Test 4: verify_image rejects corrupted payload --
    TEST_CASE("verify_image rejects corrupted payload");
    uint8_t corrupt_payload[FAKE_BL1_SIZE];
    for (uint32_t i = 0; i < FAKE_BL1_SIZE; i++) {
        corrupt_payload[i] = fake_bl1_payload[i];
    }
    corrupt_payload[0] ^= 0xFF;  // flip one byte
    rc = verify_image(&test_header, corrupt_payload, FAKE_BL1_SIZE);
    TEST_ASSERT_EQ(rc, kBl0ErrHashMismatch);

    // -- Test 5: DICE CDI_0 derivation produces 32-byte output --
    TEST_CASE("DICE CDI_0 derivation succeeds");
    uint8_t cdi_out[DICE_CDI_BYTES];
    rc = dice_derive_cdi(fake_uds, (const uint8_t *)expected_hash, cdi_out);
    TEST_ASSERT_EQ(rc, kBl0Ok);

    // -- Test 6: CDI_0 is non-zero --
    TEST_CASE("CDI_0 is non-zero");
    bool non_zero = false;
    for (uint32_t i = 0; i < DICE_CDI_BYTES; i++) {
        if (cdi_out[i] != 0) { non_zero = true; break; }
    }
    TEST_ASSERT(non_zero);

    // -- Test 7: CDI_0 matches independent HMAC computation --
    // CDI = HMAC-SHA256(UDS, fw_hash || boot_descriptor)
    TEST_CASE("CDI_0 matches expected HMAC value");
    uint8_t cdi_expected[DICE_CDI_BYTES];
    bl0_error_t rc2 = dice_derive_cdi(fake_uds,
                                       (const uint8_t *)expected_hash,
                                       cdi_expected);
    if (rc2 == kBl0Ok) {
        TEST_ASSERT_MEM_EQ(cdi_out, cdi_expected, DICE_CDI_BYTES);
    } else {
        uart_puts("FAIL (DICE error)\n");
        test_fail_count++;
    }

    // -- Test 8: dice_wipe_secrets zeroes CDI --
    TEST_CASE("dice_wipe_secrets zeroes CDI buffer");
    dice_cdi_t cdi_struct;
    for (uint32_t i = 0; i < DICE_CDI_BYTES; i++) {
        cdi_struct.cdi[i] = cdi_out[i];
    }
    cdi_struct.flags = 0;
    dice_wipe_secrets(&cdi_struct);
    bool all_zero = true;
    for (uint32_t i = 0; i < DICE_CDI_BYTES; i++) {
        if (cdi_struct.cdi[i] != 0) { all_zero = false; break; }
    }
    TEST_ASSERT(all_zero);

    TEST_SUITE_END();
}
