// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// ota_spi.c -- SPI command handler for OTA protocol
// Parses SPI command frames and routes to ota.c core functions.
//
// Command frame format (little-endian):
//   [0]       opcode (OTA_CMD_*)
//   [1..N]    payload (command-specific)
//
// Response frame format:
//   [0]       status  (ota_error_t)
//   [1..3]    reserved
//   [4..N]    payload (command-specific)

#include "ota_spi.h"
#include "ota.h"
#include "../hal/uart_drv.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

extern void *memcpy(void *dst, const void *src, unsigned int n);

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static uint32_t read_u32(const uint8_t *p) {
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

static void write_u32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v);
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

// ---------------------------------------------------------------------------
// SPI command processor
// ---------------------------------------------------------------------------

void ota_spi_process(const uint8_t *cmd, uint32_t cmd_len,
                     uint8_t *resp, uint32_t *resp_len) {
    if (cmd == NULL || resp == NULL || resp_len == NULL || cmd_len < 1) {
        if (resp_len != NULL) {
            *resp_len = 0;
        }
        return;
    }

    uint8_t opcode = cmd[0];
    ota_error_t err = kOtaOk;

    switch (opcode) {

    case OTA_CMD_BEGIN:
        // Payload: [1..4] image_size, [5..8] image_version
        if (cmd_len < 9) {
            err = kOtaErrBadArg;
        } else {
            uint32_t size = read_u32(&cmd[1]);
            uint32_t ver  = read_u32(&cmd[5]);
            err = ota_begin(size, ver);
        }
        resp[0] = (uint8_t)err;
        *resp_len = 4;
        break;

    case OTA_CMD_DATA:
        // Payload: [1..4] offset, [5..8] length, [9..] data
        if (cmd_len < 9) {
            err = kOtaErrBadArg;
        } else {
            uint32_t offset = read_u32(&cmd[1]);
            uint32_t len    = read_u32(&cmd[5]);
            if (cmd_len < 9 + len) {
                err = kOtaErrBadArg;
            } else {
                err = ota_write_chunk(&cmd[9], offset, len);
            }
        }
        resp[0] = (uint8_t)err;
        *resp_len = 4;
        break;

    case OTA_CMD_FINISH:
        err = ota_finish();
        if (err == kOtaOk) {
            err = ota_swap();
        }
        resp[0] = (uint8_t)err;
        *resp_len = 4;
        break;

    case OTA_CMD_STATUS:
        resp[0] = (uint8_t)kOtaOk;
        write_u32(&resp[4], (uint32_t)ota_get_status());
        *resp_len = 8;
        break;

    case OTA_CMD_ROLLBACK:
        err = ota_rollback();
        resp[0] = (uint8_t)err;
        *resp_len = 4;
        break;

    default:
        uart_puts("[OTA-SPI] unknown opcode\r\n");
        resp[0] = (uint8_t)kOtaErrBadArg;
        *resp_len = 4;
        break;
    }
}
