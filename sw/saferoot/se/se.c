// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// se.c — Secure Element APDU command dispatcher
// Receives APDU commands over SPI and routes to crypto/keystore handlers.

#include "se.h"
#include "se_crypto.h"
#include "se_keystore.h"
#include "../hal/hal.h"
#include "../hal/spi_drv.h"
#include "../hal/flash_drv.h"

// ---------------------------------------------------------------------------
// Forward declarations for command handlers
// ---------------------------------------------------------------------------
static void handle_get_random(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp);
static void handle_gen_key(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp);
static void handle_sign(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp);
static void handle_verify(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp);
static void handle_encrypt(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp);
static void handle_decrypt(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp);
static void handle_get_cert(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp);
static void handle_store_cert(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp);

// ---------------------------------------------------------------------------
// se_init — Initialize Secure Element subsystems
// ---------------------------------------------------------------------------
void se_init(void) {
    // Initialize key store (loads keys from OTP/Flash)
    keystore_init();

    // Initialize SPI transport (slave mode)
    spi_init();
}

// ---------------------------------------------------------------------------
// se_process_apdu — Main APDU dispatcher
// ---------------------------------------------------------------------------
void se_process_apdu(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp) {
    // Default: empty response
    resp->len = 0;
    resp->sw  = SW_INS_NOT_SUPPORT;

    // Verify CLA byte (0x80 = proprietary class for SE commands)
    if (cmd->cla != 0x80) {
        resp->sw = SW_WRONG_DATA;
        return;
    }

    // Route based on INS byte
    switch (cmd->ins) {
        case SE_CMD_GET_RANDOM:
            handle_get_random(cmd, resp);
            break;
        case SE_CMD_GEN_KEY:
            handle_gen_key(cmd, resp);
            break;
        case SE_CMD_SIGN:
            handle_sign(cmd, resp);
            break;
        case SE_CMD_VERIFY:
            handle_verify(cmd, resp);
            break;
        case SE_CMD_ENCRYPT:
            handle_encrypt(cmd, resp);
            break;
        case SE_CMD_DECRYPT:
            handle_decrypt(cmd, resp);
            break;
        case SE_CMD_GET_CERT:
            handle_get_cert(cmd, resp);
            break;
        case SE_CMD_STORE_CERT:
            handle_store_cert(cmd, resp);
            break;
        default:
            resp->sw = SW_INS_NOT_SUPPORT;
            break;
    }
}

// ---------------------------------------------------------------------------
// Command handlers
// ---------------------------------------------------------------------------

// GET RANDOM: P1=0, P2=0, Le=requested length
static void handle_get_random(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp) {
    uint16_t len = cmd->le;
    if (len == 0 || len > sizeof(resp->data)) {
        resp->sw = SW_WRONG_LENGTH;
        return;
    }
    resp->sw = se_get_random(resp->data, len);
    if (resp->sw == SW_OK) {
        resp->len = len;
    }
}

// GEN KEY: P1=slot, P2=key_type
static void handle_gen_key(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp) {
    uint8_t slot = cmd->p1;
    se_key_type_t type = (se_key_type_t)cmd->p2;

    if (slot >= SE_MAX_KEY_SLOTS) {
        resp->sw = SW_WRONG_P1P2;
        return;
    }
    resp->sw = keystore_generate(slot, type);
}

// SIGN: P1=slot, data=32-byte hash
static void handle_sign(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp) {
    if (cmd->lc != 32) {
        resp->sw = SW_WRONG_LENGTH;
        return;
    }
    if (cmd->p1 >= SE_MAX_KEY_SLOTS) {
        resp->sw = SW_WRONG_P1P2;
        return;
    }
    resp->sw = se_sign(cmd->p1, cmd->data, resp->data);
    if (resp->sw == SW_OK) {
        resp->len = 64;  // r(32) || s(32)
    }
}

// VERIFY: P1=slot, data=32-byte hash + 64-byte signature
static void handle_verify(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp) {
    if (cmd->lc != 96) {  // 32 hash + 64 sig
        resp->sw = SW_WRONG_LENGTH;
        return;
    }
    if (cmd->p1 >= SE_MAX_KEY_SLOTS) {
        resp->sw = SW_WRONG_P1P2;
        return;
    }
    const uint8_t *hash = cmd->data;
    const uint8_t *sig  = cmd->data + 32;
    resp->sw = se_verify(cmd->p1, hash, sig);
}

// ENCRYPT: P1=slot, data=plaintext (multiple of 16 bytes)
static void handle_encrypt(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp) {
    if (cmd->lc == 0 || (cmd->lc % 16) != 0 || cmd->lc > sizeof(resp->data)) {
        resp->sw = SW_WRONG_LENGTH;
        return;
    }
    if (cmd->p1 >= SE_MAX_KEY_SLOTS) {
        resp->sw = SW_WRONG_P1P2;
        return;
    }
    resp->sw = se_aes_encrypt(cmd->p1, cmd->data, resp->data, cmd->lc);
    if (resp->sw == SW_OK) {
        resp->len = cmd->lc;
    }
}

// DECRYPT: P1=slot, data=ciphertext (multiple of 16 bytes)
static void handle_decrypt(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp) {
    if (cmd->lc == 0 || (cmd->lc % 16) != 0 || cmd->lc > sizeof(resp->data)) {
        resp->sw = SW_WRONG_LENGTH;
        return;
    }
    if (cmd->p1 >= SE_MAX_KEY_SLOTS) {
        resp->sw = SW_WRONG_P1P2;
        return;
    }
    resp->sw = se_aes_decrypt(cmd->p1, cmd->data, resp->data, cmd->lc);
    if (resp->sw == SW_OK) {
        resp->len = cmd->lc;
    }
}

// GET CERT: P1=slot — read certificate from flash
// Certificate stored at Flash bank 1, page (248 + slot), max 1024 bytes
#define CERT_FLASH_BASE  0x200F8000u  // last 32KB of flash
#define CERT_MAX_SIZE    1024u

static void handle_get_cert(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp) {
    uint8_t slot = cmd->p1;
    if (slot >= SE_MAX_KEY_SLOTS) { resp->sw = SW_WRONG_P1P2; return; }
    uint32_t addr = CERT_FLASH_BASE + (slot * CERT_MAX_SIZE);
    // Read cert length from first 2 bytes
    uint32_t hdr = 0;
    flash_read(addr, &hdr, 1);
    uint16_t cert_len = hdr & 0xFFFF;
    if (cert_len == 0 || cert_len > CERT_MAX_SIZE - 2) {
        resp->sw = SW_KEY_NOT_FOUND;
        return;
    }
    // Read cert data (after 4-byte header)
    uint32_t words = (cert_len + 3) / 4;
    if (words > resp->len / 4) { resp->sw = SW_WRONG_LENGTH; return; }
    flash_read(addr + 4, (uint32_t *)resp->data, words);
    resp->len = cert_len;
    resp->sw = SW_OK;
}

// STORE CERT: P1=slot, data=DER-encoded certificate
static void handle_store_cert(const se_apdu_cmd_t *cmd, se_apdu_resp_t *resp) {
    uint8_t slot = cmd->p1;
    if (slot >= SE_MAX_KEY_SLOTS) { resp->sw = SW_WRONG_P1P2; return; }
    if (cmd->lc == 0 || cmd->lc > CERT_MAX_SIZE - 2) { resp->sw = SW_WRONG_LENGTH; return; }
    uint32_t addr = CERT_FLASH_BASE + (slot * CERT_MAX_SIZE);
    // Erase page first
    flash_erase_page(addr);
    // Write length header
    uint32_t hdr = cmd->lc;
    flash_write(addr, &hdr, 1);
    // Write cert data
    uint32_t words = (cmd->lc + 3) / 4;
    flash_write(addr + 4, (const uint32_t *)cmd->data, words);
    resp->sw = SW_OK;
}
