// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// test_tpm_pcr.c -- Integration test: TPM startup -> PCR extend -> read -> verify
// Exercises the TPM PCR subsystem end-to-end with known digests.

#include "../test/test_framework.h"
#include "../hal/hal.h"
#include "../hal/hmac_drv.h"
#include "../tpm/tpm.h"
#include "../tpm/tpm_pcr.h"

// ---------------------------------------------------------------------------
// Known test digest (32 bytes of 0x42)
// ---------------------------------------------------------------------------
static const uint8_t test_digest[TPM_PCR_SIZE] = {
    0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42,
    0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42,
    0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42,
    0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42,
};

// Second digest for chaining test (32 bytes of 0x55)
static const uint8_t test_digest_2[TPM_PCR_SIZE] = {
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
};

// Zero buffer for initial PCR comparison
static const uint8_t zero_pcr[TPM_PCR_SIZE] = {0};

void test_tpm_pcr_suite(void) {
    TEST_SUITE_START("TPM PCR");

    // -- Init TPM and PCRs --
    tpm_init();

    // -- Test 1: PCR[0] starts as all zeros --
    TEST_CASE("PCR[0] initial value is all zeros");
    uint8_t pcr_val[TPM_PCR_SIZE];
    uint32_t rc = tpm_pcr_read(0, pcr_val);
    TEST_ASSERT(rc == TPM_RC_SUCCESS);

    TEST_CASE("PCR[0] is zero after init");
    TEST_ASSERT_MEM_EQ(pcr_val, zero_pcr, TPM_PCR_SIZE);

    // -- Test 2: Extend PCR[0] with known digest --
    TEST_CASE("PCR extend returns success");
    rc = tpm_pcr_extend(0, test_digest);
    TEST_ASSERT_EQ(rc, TPM_RC_SUCCESS);

    // -- Test 3: Read PCR[0], verify = SHA-256(zeros || test_digest) --
    TEST_CASE("PCR[0] = SHA-256(zeros || digest)");
    uint8_t concat[TPM_PCR_SIZE * 2];
    for (uint32_t i = 0; i < TPM_PCR_SIZE; i++) {
        concat[i] = 0;  // PCR was zero
    }
    for (uint32_t i = 0; i < TPM_PCR_SIZE; i++) {
        concat[TPM_PCR_SIZE + i] = test_digest[i];
    }
    uint32_t expected_hash[8];
    sha256(concat, TPM_PCR_SIZE * 2, expected_hash);

    rc = tpm_pcr_read(0, pcr_val);
    if (rc == TPM_RC_SUCCESS) {
        TEST_ASSERT_MEM_EQ(pcr_val, expected_hash, TPM_PCR_SIZE);
    } else {
        uart_puts("FAIL (pcr_read error)\n");
        test_fail_count++;
    }

    // -- Test 4: Extend again, verify chaining --
    TEST_CASE("PCR chaining: second extend");
    rc = tpm_pcr_extend(0, test_digest_2);
    TEST_ASSERT_EQ(rc, TPM_RC_SUCCESS);

    // Expected: SHA-256(prev_pcr || test_digest_2)
    TEST_CASE("PCR[0] after second extend matches chain");
    uint8_t concat2[TPM_PCR_SIZE * 2];
    for (uint32_t i = 0; i < TPM_PCR_SIZE; i++) {
        concat2[i] = ((const uint8_t *)expected_hash)[i];
    }
    for (uint32_t i = 0; i < TPM_PCR_SIZE; i++) {
        concat2[TPM_PCR_SIZE + i] = test_digest_2[i];
    }
    uint32_t chain_hash[8];
    sha256(concat2, TPM_PCR_SIZE * 2, chain_hash);

    rc = tpm_pcr_read(0, pcr_val);
    if (rc == TPM_RC_SUCCESS) {
        TEST_ASSERT_MEM_EQ(pcr_val, chain_hash, TPM_PCR_SIZE);
    } else {
        uart_puts("FAIL (pcr_read error)\n");
        test_fail_count++;
    }

    // -- Test 5: PCR[16] is resettable --
    TEST_CASE("PCR[16] reset succeeds (resettable range)");
    tpm_pcr_extend(16, test_digest);  // extend first so it's non-zero
    rc = tpm_pcr_reset(16);
    TEST_ASSERT_EQ(rc, TPM_RC_SUCCESS);

    TEST_CASE("PCR[16] is zero after reset");
    rc = tpm_pcr_read(16, pcr_val);
    if (rc == TPM_RC_SUCCESS) {
        TEST_ASSERT_MEM_EQ(pcr_val, zero_pcr, TPM_PCR_SIZE);
    } else {
        uart_puts("FAIL (pcr_read error)\n");
        test_fail_count++;
    }

    // -- Test 6: PCR[0] reset should fail (not resettable) --
    TEST_CASE("PCR[0] reset fails (not resettable)");
    rc = tpm_pcr_reset(0);
    TEST_ASSERT_EQ(rc, TPM_RC_PCR);

    // -- Test 7: Invalid PCR index returns error --
    TEST_CASE("PCR extend on invalid index returns error");
    rc = tpm_pcr_extend(TPM_NUM_PCRS, test_digest);
    TEST_ASSERT_EQ(rc, TPM_RC_VALUE);

    TEST_SUITE_END();
}
