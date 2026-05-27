// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// spi_drv.h — SPI Device driver for SafeRoot (OpenTitan SPI Device IP)
// Operates as SPI slave with upload (command FIFO) interface.

#ifndef SAFEROOT_SPI_DRV_H_
#define SAFEROOT_SPI_DRV_H_

#include "hal.h"

// Initialize SPI device in flash emulation mode
hal_status_t spi_init(void);

// Check if an uploaded command is available in the command FIFO
bool spi_cmd_available(void);

// Read one command from the upload command FIFO (8-bit opcode)
uint8_t spi_read_cmd(void);

// Read the uploaded address from the address FIFO
uint32_t spi_read_addr(void);

// Write data to the read buffer (egress) for SPI host to read
hal_status_t spi_write_read_buffer(uint32_t offset, const uint32_t *data,
                                   uint32_t word_count);

// Read data from the payload buffer (ingress, uploaded by SPI host)
hal_status_t spi_read_payload(uint32_t offset, uint32_t *data,
                              uint32_t word_count);

// Set the JEDEC ID that the SPI device reports
void spi_set_jedec_id(uint16_t id, uint8_t manufacturer, uint8_t cc_num);

// Set flash status register value
void spi_set_flash_status(uint32_t status);

#endif  // SAFEROOT_SPI_DRV_H_
