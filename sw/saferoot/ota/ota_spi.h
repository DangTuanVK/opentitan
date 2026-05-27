// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// ota_spi.h -- SPI command handler for OTA protocol
// Routes OTA commands received from host SoC to ota.c functions.

#ifndef SAFEROOT_OTA_SPI_H_
#define SAFEROOT_OTA_SPI_H_

#include <stdint.h>

// Process an OTA SPI command.
// cmd:      incoming command buffer (opcode + payload)
// cmd_len:  length of command buffer in bytes
// resp:     response buffer to fill
// resp_len: on entry, max response buffer size; on exit, actual response size
void ota_spi_process(const uint8_t *cmd, uint32_t cmd_len,
                     uint8_t *resp, uint32_t *resp_len);

#endif  // SAFEROOT_OTA_SPI_H_
