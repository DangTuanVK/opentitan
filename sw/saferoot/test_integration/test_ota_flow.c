// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// test_ota_flow.c -- Integration test: OTA begin -> write chunks -> finish
// Exercises the full OTA update pipeline with a fake firmware image.

#include "../test/test_framework.h"
#include "../hal/hal.h"
#include "../hal/hmac_drv.h"
#include "../hal/flash_drv.h"
#include "../ota/ota.h"

// ---------------------------------------------------------------------------
// Fake firmware image: 256 bytes of known data
// ---------------------------------------------------------------------------
#define FAKE_FW_SIZE   256U
#define FAKE_FW_VERSION  5U
#define CHUNK_SIZE       64U

static uint8_t fake_firmware[FAKE_FW_SIZE] __attribute__((aligned(4)));

static void fill_fake_firmware(void) {
    for (uint32_t i = 0; i < FAKE_FW_SIZE; i++) {
        fake_firmware[i] = (uint8_t)(i & 0xFF);
    }
}

void test_ota_flow_suite(void) {
    TEST_SUITE_START("OTA Flow");

    fill_fake_firmware();

    // -- Test 1: OTA init --
    TEST_CASE("ota_init succeeds");
    ota_error_t rc = ota_init();
    TEST_ASSERT_EQ(rc, kOtaOk);

    // -- Test 2: ota_begin with valid version --
    TEST_CASE("ota_begin with version 5");
    rc = ota_begin(FAKE_FW_SIZE, FAKE_FW_VERSION);
    TEST_ASSERT_EQ(rc, kOtaOk);

    // -- Test 3: State should be Receiving after begin --
    TEST_CASE("state is Receiving after ota_begin");
    ota_state_t state = ota_get_status();
    TEST_ASSERT_EQ(state, kOtaStateReceiving);

    // -- Test 4: Write firmware in chunks --
    TEST_CASE("write 4 chunks of 64 bytes");
    bool write_ok = true;
    for (uint32_t offset = 0; offset < FAKE_FW_SIZE; offset += CHUNK_SIZE) {
        rc = ota_write_chunk(&fake_firmware[offset], offset, CHUNK_SIZE);
        if (rc != kOtaOk) {
            write_ok = false;
            break;
        }
    }
    TEST_ASSERT(write_ok);

    // -- Test 5: ota_finish verifies hash --
    TEST_CASE("ota_finish succeeds (hash match)");
    rc = ota_finish();
    TEST_ASSERT_EQ(rc, kOtaOk);

    // -- Test 6: State should be Done after finish --
    TEST_CASE("state is Done after ota_finish");
    state = ota_get_status();
    TEST_ASSERT_EQ(state, kOtaStateDone);

    // -- Test 7: Anti-rollback -- lower version should fail --
    TEST_CASE("ota_begin with lower version fails (anti-rollback)");
    rc = ota_begin(FAKE_FW_SIZE, FAKE_FW_VERSION - 1);
    TEST_ASSERT_EQ(rc, kOtaErrAntiRollback);

    // -- Test 8: ota_begin with zero size fails --
    TEST_CASE("ota_begin with zero size fails");
    rc = ota_begin(0, FAKE_FW_VERSION + 1);
    TEST_ASSERT_EQ(rc, kOtaErrBadArg);

    // -- Test 9: ota_begin with oversized image fails --
    TEST_CASE("ota_begin with oversized image fails");
    rc = ota_begin(OTA_MAX_IMAGE_SIZE + 1, FAKE_FW_VERSION + 1);
    TEST_ASSERT_EQ(rc, kOtaErrBadSize);

    // -- Test 10: Write chunk in wrong state fails --
    // After a successful finish, state is Done -- writing should fail
    TEST_CASE("write_chunk in Done state fails");
    rc = ota_write_chunk(fake_firmware, 0, CHUNK_SIZE);
    TEST_ASSERT_EQ(rc, kOtaErrBadState);

    TEST_SUITE_END();
}
