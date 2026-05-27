// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// Heartbeat verification module for SafeRoot chip.
// Uses HMAC IP at 0x40010000 to authenticate heartbeat messages
// received from the host over SPI.

#ifndef SAFEROOT_HEARTBEAT_H_
#define SAFEROOT_HEARTBEAT_H_

#include <stdint.h>
#include <stdbool.h>

// ----------------------------------------------------------------
// HMAC base address (SafeRoot memory map)
// ----------------------------------------------------------------
#define HMAC_BASE  0x40010000u

// ----------------------------------------------------------------
// HMAC register offsets (OpenTitan hmac IP)
// ----------------------------------------------------------------
#define HMAC_CFG          0x00u  // Configuration (SHA-256 / HMAC mode)
#define HMAC_CMD          0x04u  // Command (hash_start, hash_process, hash_done)
#define HMAC_STATUS       0x08u  // Status (fifo_full, fifo_empty, etc.)
#define HMAC_ERR_CODE     0x0Cu  // Error code
#define HMAC_WIPE_SECRET  0x10u  // Wipe internal secret state
#define HMAC_KEY_0        0x14u  // Key word 0 (MSW)
#define HMAC_KEY_7        0x30u  // Key word 7 (LSW) -- 8 words total
#define HMAC_DIGEST_0     0x34u  // Digest word 0
#define HMAC_DIGEST_7     0x50u  // Digest word 7
#define HMAC_MSG_LENGTH_LO 0x54u // Message length low 32 bits
#define HMAC_MSG_LENGTH_HI 0x58u // Message length high 32 bits
#define HMAC_MSG_FIFO     0x800u // Message FIFO (write-only window)

// ----------------------------------------------------------------
// HMAC_CFG bits
// ----------------------------------------------------------------
#define HMAC_CFG_HMAC_EN   (1u << 0)  // Enable HMAC (vs plain SHA-256)
#define HMAC_CFG_SHA_EN    (1u << 1)  // Enable SHA engine
#define HMAC_CFG_ENDIAN    (1u << 2)  // Endian swap
#define HMAC_CFG_DIGEST_SWAP (1u << 3)

// ----------------------------------------------------------------
// HMAC_CMD bits
// ----------------------------------------------------------------
#define HMAC_CMD_START     (1u << 0)
#define HMAC_CMD_PROCESS   (1u << 1)
#define HMAC_CMD_DONE      (1u << 2)

// ----------------------------------------------------------------
// HMAC_STATUS bits
// ----------------------------------------------------------------
#define HMAC_STATUS_FIFO_FULL  (1u << 0)
#define HMAC_STATUS_FIFO_EMPTY (1u << 1)

// ----------------------------------------------------------------
// GPIO pins used by heartbeat
// ----------------------------------------------------------------
#define GPIO_BASE       0x40210000u
#define GPIO_DATA_IN    0x00u   // Direct input register
#define GPIO_DIRECT_OUT 0x04u   // Direct output register

#define MIO_WAKE_PIN     7u     // Host asserts WAKE to signal heartbeat ready
#define MIO_FAILSAFE_PIN 6u
#define MIO_RESET_N_PIN  5u
#define MIO_ALERT_N_PIN  4u

// ----------------------------------------------------------------
// Heartbeat HMAC digest size (SHA-256 = 32 bytes)
// ----------------------------------------------------------------
#define HEARTBEAT_DIGEST_BYTES  32u
#define HEARTBEAT_DIGEST_WORDS   8u

// ----------------------------------------------------------------
// Return codes
// ----------------------------------------------------------------
#define HEARTBEAT_OK            0
#define HEARTBEAT_ERR_VERIFY   -1  // HMAC mismatch
#define HEARTBEAT_ERR_NO_WAKE  -2  // WAKE pin not asserted
#define HEARTBEAT_ERR_KEY      -3  // Key not configured

// ----------------------------------------------------------------
// Public API
// ----------------------------------------------------------------

/**
 * Initialise the heartbeat subsystem with an HMAC session key.
 *
 * @param session_key  Pointer to key bytes (up to 32 bytes / 256 bits).
 * @param key_len      Length of the key in bytes (must be <= 32).
 */
void heartbeat_init(const uint8_t *session_key, uint32_t key_len);

/**
 * Verify an HMAC-SHA256 heartbeat message.
 *
 * @param hmac_msg       Pointer to the message payload.
 * @param msg_len        Length of the message in bytes.
 * @param expected_hmac  Pointer to the expected 32-byte HMAC digest.
 * @return HEARTBEAT_OK on match, HEARTBEAT_ERR_VERIFY on mismatch.
 */
int heartbeat_verify(const uint8_t *hmac_msg, uint32_t msg_len,
                     const uint8_t *expected_hmac);

/**
 * Poll-based heartbeat check -- called from the main loop.
 *
 * Reads the WAKE pin; if asserted, reads a heartbeat frame from the
 * SPI mailbox, verifies the HMAC, and pets the watchdog on success.
 *
 * @return HEARTBEAT_OK if heartbeat verified (or WAKE not asserted),
 *         negative error code on failure.
 */
int heartbeat_check(void);

#endif  // SAFEROOT_HEARTBEAT_H_
