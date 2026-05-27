// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// Heartbeat HMAC verification implementation for SafeRoot chip.
// Bare-metal, rv32imc, no OS.

#include "heartbeat.h"
#include "wdt.h"
#include "../hal/hal.h"  // REG32() macro

// ----------------------------------------------------------------
// Internal state
// ----------------------------------------------------------------

// HMAC key stored as 8 x 32-bit words (SHA-256 key schedule).
static uint32_t hmac_key[HEARTBEAT_DIGEST_WORDS];
static bool     key_loaded = false;

// ----------------------------------------------------------------
// SPI mailbox stub
// The real implementation reads from the SPI device IP.  Here we
// define the expected heartbeat frame layout so the rest of the
// driver can be compiled and linked.
// ----------------------------------------------------------------

// Heartbeat frame layout received over SPI:
//   [0..msg_len-1]  payload
//   [msg_len..+31]  HMAC-SHA256 digest
#define HEARTBEAT_MAX_MSG_LEN  64u

// External SPI read -- provided by the SPI driver.
// Weak stub: returns -1 (no data) until SPI driver is linked.
__attribute__((weak))
int spi_mailbox_read(uint8_t *buf, uint32_t max_len, uint32_t *out_len) {
    (void)buf; (void)max_len;
    if (out_len) *out_len = 0;
    return -1;
}

// ----------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------

static inline uint32_t hmac_read(uint32_t offset) {
    return REG32(HMAC_BASE + offset);
}

static inline void hmac_write(uint32_t offset, uint32_t value) {
    REG32(HMAC_BASE + offset) = value;
}

/**
 * Wait until the HMAC FIFO is not full before writing.
 */
static inline void hmac_fifo_wait(void) {
    while (hmac_read(HMAC_STATUS) & HMAC_STATUS_FIFO_FULL) {
        // spin
    }
}

/**
 * Load the session key into the HMAC key registers.
 */
static void hmac_load_key(void) {
    for (uint32_t i = 0; i < HEARTBEAT_DIGEST_WORDS; i++) {
        hmac_write(HMAC_KEY_0 + (i * 4u), hmac_key[i]);
    }
}

/**
 * Constant-time comparison of two 32-byte digests.
 * Returns 0 if equal, non-zero otherwise.
 */
static int digest_compare(const uint8_t *a, const uint8_t *b, uint32_t len) {
    uint8_t diff = 0;
    for (uint32_t i = 0; i < len; i++) {
        diff |= a[i] ^ b[i];
    }
    return (int)diff;
}

// ----------------------------------------------------------------
// heartbeat_init
// ----------------------------------------------------------------
void heartbeat_init(const uint8_t *session_key, uint32_t key_len) {
    // Zero-fill the key words.
    for (uint32_t i = 0; i < HEARTBEAT_DIGEST_WORDS; i++) {
        hmac_key[i] = 0;
    }

    // Copy key bytes into the word array (big-endian packing to match
    // the HMAC IP key register layout).
    if (key_len > HEARTBEAT_DIGEST_BYTES) {
        key_len = HEARTBEAT_DIGEST_BYTES;
    }

    for (uint32_t i = 0; i < key_len; i++) {
        uint32_t word_idx = i / 4u;
        uint32_t byte_pos = 3u - (i % 4u);  // big-endian within word
        hmac_key[word_idx] |= ((uint32_t)session_key[i]) << (byte_pos * 8u);
    }

    key_loaded = true;
}

// ----------------------------------------------------------------
// heartbeat_verify
// ----------------------------------------------------------------
int heartbeat_verify(const uint8_t *hmac_msg, uint32_t msg_len,
                     const uint8_t *expected_hmac) {
    if (!key_loaded) {
        return HEARTBEAT_ERR_KEY;
    }

    // 1. Configure the HMAC block for HMAC-SHA256.
    hmac_write(HMAC_CFG, HMAC_CFG_HMAC_EN | HMAC_CFG_SHA_EN |
                          HMAC_CFG_ENDIAN | HMAC_CFG_DIGEST_SWAP);

    // 2. Load the key.
    hmac_load_key();

    // 3. Issue hash_start command.
    hmac_write(HMAC_CMD, HMAC_CMD_START);

    // 4. Feed message bytes into the FIFO, 4 bytes at a time.
    uint32_t full_words = msg_len / 4u;
    uint32_t remaining  = msg_len % 4u;

    for (uint32_t i = 0; i < full_words; i++) {
        hmac_fifo_wait();
        uint32_t word = ((uint32_t)hmac_msg[i * 4 + 0] << 24) |
                        ((uint32_t)hmac_msg[i * 4 + 1] << 16) |
                        ((uint32_t)hmac_msg[i * 4 + 2] <<  8) |
                        ((uint32_t)hmac_msg[i * 4 + 3] <<  0);
        hmac_write(HMAC_MSG_FIFO, word);
    }

    // Handle trailing bytes (pack into MSB-first word, zero-padded).
    if (remaining > 0) {
        hmac_fifo_wait();
        uint32_t word = 0;
        for (uint32_t j = 0; j < remaining; j++) {
            word |= ((uint32_t)hmac_msg[full_words * 4 + j])
                    << ((3u - j) * 8u);
        }
        hmac_write(HMAC_MSG_FIFO, word);
    }

    // 5. Signal hash_process then hash_done.
    hmac_write(HMAC_CMD, HMAC_CMD_PROCESS);
    hmac_write(HMAC_CMD, HMAC_CMD_DONE);

    // 6. Wait for the digest to be ready (fifo_empty && not busy).
    //    In the OpenTitan HMAC, the digest registers are valid once
    //    hash_done completes.  Poll HMAC_STATUS until fifo is empty.
    while (!(hmac_read(HMAC_STATUS) & HMAC_STATUS_FIFO_EMPTY)) {
        // spin
    }

    // 7. Read the computed digest.
    uint8_t computed[HEARTBEAT_DIGEST_BYTES];
    for (uint32_t i = 0; i < HEARTBEAT_DIGEST_WORDS; i++) {
        uint32_t d = hmac_read(HMAC_DIGEST_0 + (i * 4u));
        computed[i * 4 + 0] = (uint8_t)(d >> 24);
        computed[i * 4 + 1] = (uint8_t)(d >> 16);
        computed[i * 4 + 2] = (uint8_t)(d >>  8);
        computed[i * 4 + 3] = (uint8_t)(d >>  0);
    }

    // 8. Wipe the HMAC internal state after use.
    hmac_write(HMAC_WIPE_SECRET, 1u);

    // 9. Constant-time comparison.
    if (digest_compare(computed, expected_hmac, HEARTBEAT_DIGEST_BYTES) != 0) {
        return HEARTBEAT_ERR_VERIFY;
    }

    return HEARTBEAT_OK;
}

// ----------------------------------------------------------------
// heartbeat_check
// ----------------------------------------------------------------
int heartbeat_check(void) {
    // 1. Check WAKE pin (MIO[7]) -- host asserts it when a heartbeat
    //    frame is available in the SPI mailbox.
    uint32_t gpio_in = REG32(GPIO_BASE + GPIO_DATA_IN);
    if (!(gpio_in & (1u << MIO_WAKE_PIN))) {
        // WAKE not asserted -- nothing to do.
        return HEARTBEAT_OK;
    }

    // 2. Read the heartbeat frame from the SPI mailbox.
    uint8_t  frame[HEARTBEAT_MAX_MSG_LEN + HEARTBEAT_DIGEST_BYTES];
    uint32_t frame_len = 0;
    int rc = spi_mailbox_read(frame, sizeof(frame), &frame_len);
    if (rc != 0 || frame_len <= HEARTBEAT_DIGEST_BYTES) {
        return HEARTBEAT_ERR_VERIFY;
    }

    // Split the frame into message and expected HMAC.
    uint32_t       msg_len       = frame_len - HEARTBEAT_DIGEST_BYTES;
    const uint8_t *msg_ptr       = frame;
    const uint8_t *expected_ptr  = frame + msg_len;

    // 3. Verify the HMAC.
    rc = heartbeat_verify(msg_ptr, msg_len, expected_ptr);
    if (rc != HEARTBEAT_OK) {
        // Verification failed -- do NOT pet the watchdog.
        // Assert FAILSAFE pin (MIO[6]) to signal the error externally.
        uint32_t gpio_out = REG32(GPIO_BASE + GPIO_DIRECT_OUT);
        gpio_out |= (1u << MIO_FAILSAFE_PIN);
        REG32(GPIO_BASE + GPIO_DIRECT_OUT) = gpio_out;
        return rc;
    }

    // 4. Heartbeat is authentic -- pet the watchdog.
    wdt_pet();

    return HEARTBEAT_OK;
}
