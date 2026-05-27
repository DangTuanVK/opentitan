// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// spi_drv.c — SPI Device driver for SafeRoot (OpenTitan SPI Device IP)
// Register offsets from spi_device_reg_pkg.sv
//
// The SPI device operates as a slave. The host sends commands over SPI,
// which are uploaded into FIFOs. Firmware processes them and provides
// read data via the egress SRAM buffer.

#include "spi_drv.h"

// ---------------------------------------------------------------------------
// SPI Device register offsets (from spi_device_reg_pkg.sv)
// ---------------------------------------------------------------------------
#define SPI_INTR_STATE         0x00
#define SPI_INTR_ENABLE        0x04
#define SPI_INTR_TEST          0x08
#define SPI_ALERT_TEST         0x0C
#define SPI_CONTROL            0x10  // Control register
#define SPI_CFG                0x14  // Configuration
#define SPI_STATUS             0x18  // Status (RO)
#define SPI_INTERCEPT_EN       0x1C
#define SPI_ADDR_MODE          0x20
#define SPI_LAST_READ_ADDR     0x24
#define SPI_FLASH_STATUS       0x28  // Flash status register
#define SPI_JEDEC_CC           0x2C  // JEDEC continuation code
#define SPI_JEDEC_ID           0x30  // JEDEC manufacturer + device ID
#define SPI_READ_THRESHOLD     0x34
#define SPI_MAILBOX_ADDR       0x38
#define SPI_UPLOAD_STATUS      0x3C  // Upload FIFO status
#define SPI_UPLOAD_STATUS2     0x40
#define SPI_UPLOAD_CMDFIFO     0x44  // Upload command FIFO (RO)
#define SPI_UPLOAD_ADDRFIFO    0x48  // Upload address FIFO (RO)

// SRAM windows (word-addressable from base)
// Egress buffer (read buffer for host): offset 0x1000, 2048 words
// Ingress buffer (payload from host):   offset 0x1800, 256 words
#define SPI_EGRESS_BUFFER      0x1000
#define SPI_INGRESS_BUFFER     0x1800

// ---------------------------------------------------------------------------
// CONTROL register bit definitions
// ---------------------------------------------------------------------------
// [0] ABORT — abort current SPI transaction
// [4] MODE — 0: flash mode, 1: passthrough mode
#define SPI_CONTROL_ABORT      BIT(0)

// ---------------------------------------------------------------------------
// STATUS register bit definitions
// ---------------------------------------------------------------------------
// [0]   CSB — current chip-select state (active low)
// [1]   TPM_CSB — current TPM chip-select state
// [5:4] UPLOAD_CMDFIFO_DEPTH
// [8:6] UPLOAD_ADDRFIFO_DEPTH
// [15:9] UPLOAD_CMDFIFO_NOTEMPTY (via INTR_STATE bit 0)
#define SPI_STATUS_CSB              BIT(0)

// ---------------------------------------------------------------------------
// UPLOAD_STATUS register bit definitions
// ---------------------------------------------------------------------------
// [4:0]  CMDFIFO_DEPTH — number of entries in command FIFO
// [12:8] ADDRFIFO_DEPTH — number of entries in address FIFO
#define SPI_UPLOAD_CMDFIFO_DEPTH_MASK   0x1F
#define SPI_UPLOAD_ADDRFIFO_DEPTH_MASK  0x1F
#define SPI_UPLOAD_ADDRFIFO_DEPTH_SHIFT 8

// ---------------------------------------------------------------------------
// INTR_STATE bits
// ---------------------------------------------------------------------------
#define SPI_INTR_UPLOAD_CMDFIFO_NOT_EMPTY  BIT(0)
#define SPI_INTR_UPLOAD_PAYLOAD_NOT_EMPTY  BIT(1)
#define SPI_INTR_UPLOAD_PAYLOAD_OVERFLOW   BIT(2)

// ---------------------------------------------------------------------------
// JEDEC register fields
// ---------------------------------------------------------------------------
// JEDEC_CC: [7:0] cc (continuation code), [15:8] num_cc
// JEDEC_ID: [7:0] manufacturer_id, [23:8] device_id

#define SPI_BASE SAFEROOT_SPI_DEVICE_BASE

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

hal_status_t spi_init(void) {
    // Clear pending interrupts
    REG32_WRITE(SPI_BASE + SPI_INTR_STATE, 0xFFFFFFFF);
    // Disable interrupts (polling mode)
    REG32_WRITE(SPI_BASE + SPI_INTR_ENABLE, 0);
    // Set flash mode (MODE=0), no abort
    REG32_WRITE(SPI_BASE + SPI_CONTROL, 0);
    return kHalOk;
}

bool spi_cmd_available(void) {
    uint32_t st = REG32_READ(SPI_BASE + SPI_UPLOAD_STATUS);
    return (st & SPI_UPLOAD_CMDFIFO_DEPTH_MASK) > 0;
}

uint8_t spi_read_cmd(void) {
    return (uint8_t)(REG32_READ(SPI_BASE + SPI_UPLOAD_CMDFIFO) & 0xFF);
}

uint32_t spi_read_addr(void) {
    return REG32_READ(SPI_BASE + SPI_UPLOAD_ADDRFIFO);
}

hal_status_t spi_write_read_buffer(uint32_t offset, const uint32_t *data,
                                   uint32_t word_count) {
    if (data == (void *)0) return kHalBadArg;
    for (uint32_t i = 0; i < word_count; i++) {
        REG32_WRITE(SPI_BASE + SPI_EGRESS_BUFFER + offset + (i * 4), data[i]);
    }
    return kHalOk;
}

hal_status_t spi_read_payload(uint32_t offset, uint32_t *data,
                              uint32_t word_count) {
    if (data == (void *)0) return kHalBadArg;
    for (uint32_t i = 0; i < word_count; i++) {
        data[i] = REG32_READ(SPI_BASE + SPI_INGRESS_BUFFER + offset + (i * 4));
    }
    return kHalOk;
}

void spi_set_jedec_id(uint16_t id, uint8_t manufacturer, uint8_t cc_num) {
    // JEDEC_CC: [7:0] = continuation code (0x7F typical), [15:8] = count
    REG32_WRITE(SPI_BASE + SPI_JEDEC_CC,
                (uint32_t)cc_num << 8 | 0x7F);
    // JEDEC_ID: [7:0] = manufacturer, [23:8] = device ID
    REG32_WRITE(SPI_BASE + SPI_JEDEC_ID,
                ((uint32_t)id << 8) | (uint32_t)manufacturer);
}

void spi_set_flash_status(uint32_t status) {
    REG32_WRITE(SPI_BASE + SPI_FLASH_STATUS, status);
}
