// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// se.h — Secure Element API header
// Provides APDU-based cryptographic services over SPI (ISO 7816 style).

#ifndef SAFEROOT_SE_H_
#define SAFEROOT_SE_H_

#include <stdint.h>
#include <stdbool.h>

// ---------------------------------------------------------------------------
// APDU command structure (ISO 7816-4 inspired)
// ---------------------------------------------------------------------------
typedef struct {
    uint8_t  cla;           // Class byte
    uint8_t  ins;           // Instruction byte
    uint8_t  p1;            // Parameter 1
    uint8_t  p2;            // Parameter 2
    uint16_t lc;            // Length of command data
    uint8_t  data[256];     // Command data payload
    uint16_t le;            // Expected response length
} se_apdu_cmd_t;

typedef struct {
    uint8_t  data[256];     // Response data payload
    uint16_t len;           // Actual response data length
    uint16_t sw;            // Status word (e.g. 0x9000)
} se_apdu_resp_t;

// ---------------------------------------------------------------------------
// SE command codes (INS byte values)
// ---------------------------------------------------------------------------
#define SE_CMD_GET_RANDOM   0x84  // Generate random bytes
#define SE_CMD_GEN_KEY      0x46  // Generate key pair in slot
#define SE_CMD_SIGN         0x2A  // ECDSA-P256 sign
#define SE_CMD_VERIFY       0x2B  // ECDSA-P256 verify
#define SE_CMD_ENCRYPT      0x86  // AES-256 encrypt
#define SE_CMD_DECRYPT      0x87  // AES-256 decrypt
#define SE_CMD_GET_CERT     0xCA  // Read certificate from slot
#define SE_CMD_STORE_CERT   0xDA  // Store certificate to slot

// ---------------------------------------------------------------------------
// Status words (SW1-SW2)
// ---------------------------------------------------------------------------
#define SW_OK               0x9000  // Command completed successfully
#define SW_WRONG_LENGTH     0x6700  // Wrong Lc or Le
#define SW_SECURITY         0x6982  // Security condition not satisfied
#define SW_WRONG_DATA       0x6A80  // Incorrect data parameters
#define SW_WRONG_P1P2       0x6A86  // Incorrect P1/P2
#define SW_INS_NOT_SUPPORT  0x6D00  // Instruction not supported
#define SW_INTERNAL_ERROR   0x6F00  // Internal error
#define SW_KEY_NOT_FOUND    0x6A88  // Referenced key not found

// ---------------------------------------------------------------------------
// Key types and slots
// ---------------------------------------------------------------------------
#define SE_MAX_KEY_SLOTS    8

typedef enum {
    kKeyTypeNone     = 0x00,
    kKeyTypeAes256   = 0x01,  // 256-bit AES symmetric key
    kKeyTypeEcdsaP256 = 0x02, // ECDSA NIST P-256 key pair
    kKeyTypeHmac256  = 0x03,  // 256-bit HMAC key
} se_key_type_t;

#define SE_KEY_FLAG_PERSISTENT  0x01  // Key stored in Flash
#define SE_KEY_FLAG_EXPORTABLE  0x02  // Key may be exported
#define SE_KEY_FLAG_ACTIVE      0x80  // Slot is in use

typedef struct {
    uint8_t       id;         // Slot index (0 .. SE_MAX_KEY_SLOTS-1)
    se_key_type_t type;       // Key algorithm type
    uint8_t       flags;      // SE_KEY_FLAG_* bitmask
    uint32_t      key[8];     // Up to 256-bit key material (private/symmetric)
    uint32_t      pub_x[8];   // ECDSA public key X (P-256 = 256 bits)
    uint32_t      pub_y[8];   // ECDSA public key Y
} se_key_t;

// ---------------------------------------------------------------------------
// SE lifecycle API
// ---------------------------------------------------------------------------

// Initialize the Secure Element (key store, SPI transport, peripherals).
void se_init(void);

// Process one APDU command and produce a response.
void se_process_apdu(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp);

#endif  // SAFEROOT_SE_H_
