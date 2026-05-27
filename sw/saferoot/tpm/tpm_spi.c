// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// tpm_spi.c — TPM over SPI transport layer for SafeRoot
// Implements TCG PC Client Platform TPM Profile (PTP) SPI interface.
// Uses SafeRoot HAL SPI slave driver to receive commands from host.

#include "tpm_spi.h"
#include "tpm.h"
#include "../hal/spi_drv.h"
#include "../hal/uart_drv.h"
#include "../hal/hal.h"

// ---------------------------------------------------------------------------
// TPM locality registers (simulated in SRAM)
// ---------------------------------------------------------------------------
static uint8_t tpm_access_reg  = TPM_ACCESS_VALID | TPM_ACCESS_ESTABLISH;
static uint8_t tpm_sts_reg     = TPM_STS_CMD_READY;

// ---------------------------------------------------------------------------
// Command / response buffers
// ---------------------------------------------------------------------------
static uint8_t  cmd_buf[TPM_CMD_BUF_SIZE]   __attribute__((aligned(4)));
static uint8_t  resp_buf[TPM_RESP_BUF_SIZE]  __attribute__((aligned(4)));
static uint32_t cmd_pos  = 0;  // write cursor into cmd_buf
static uint32_t resp_pos = 0;  // read cursor into resp_buf
static uint32_t resp_len = 0;  // total response length

// ---------------------------------------------------------------------------
// SPI protocol states
// ---------------------------------------------------------------------------
typedef enum {
    kSpiIdle = 0,
    kSpiReceiving,     // Host is writing command bytes
    kSpiProcessing,    // TPM is processing command
    kSpiResponding,    // Host is reading response bytes
} spi_state_t;

static spi_state_t spi_state = kSpiIdle;

// ---------------------------------------------------------------------------
// tpm_spi_init — Initialize SPI transport
// ---------------------------------------------------------------------------
void tpm_spi_init(void) {
    spi_init();

    // Set SPI read buffer with TPM register defaults
    // DID_VID: SafeRoot vendor 0x5346 ("SF"), device 0x5254 ("RT")
    uint32_t did_vid = 0x52545346U;
    spi_write_read_buffer(TPM_REG_DID_VID, &did_vid, 1);

    // Interface capability: TIS 1.3, FIFO interface
    uint32_t intf_cap = 0x0000001FU;
    spi_write_read_buffer(TPM_REG_INTF_CAP, &intf_cap, 1);

    cmd_pos  = 0;
    resp_pos = 0;
    resp_len = 0;
    spi_state = kSpiIdle;

    uart_puts("[TPM-SPI] Transport initialized\r\n");
}

// ---------------------------------------------------------------------------
// Handle a register write from host
// ---------------------------------------------------------------------------
static void handle_reg_write(uint32_t reg_addr, const uint8_t *data,
                             uint32_t len) {
    switch (reg_addr) {
        case TPM_REG_ACCESS:
            if (data[0] & TPM_ACCESS_REQUEST) {
                tpm_access_reg |= TPM_ACCESS_ACTIVE;
            }
            if (data[0] & TPM_ACCESS_ACTIVE) {
                // Host is releasing locality
                tpm_access_reg &= ~TPM_ACCESS_ACTIVE;
            }
            break;

        case TPM_REG_STS:
            if (data[0] & TPM_STS_CMD_READY) {
                // Host signals ready to send new command
                cmd_pos   = 0;
                resp_pos  = 0;
                resp_len  = 0;
                spi_state = kSpiReceiving;
                tpm_sts_reg = TPM_STS_CMD_READY | TPM_STS_EXPECT;
            }
            if (data[0] & TPM_STS_GO) {
                // Host says command is complete — process it
                spi_state = kSpiProcessing;
                tpm_sts_reg &= ~TPM_STS_EXPECT;
            }
            if (data[0] & TPM_STS_RESP_RETRY) {
                // Re-send response from beginning
                resp_pos = 0;
            }
            break;

        case TPM_REG_DATA_FIFO:
            // Host is writing command bytes
            if (spi_state == kSpiReceiving && cmd_pos < TPM_CMD_BUF_SIZE) {
                uint32_t copy = len;
                if (cmd_pos + copy > TPM_CMD_BUF_SIZE) {
                    copy = TPM_CMD_BUF_SIZE - cmd_pos;
                }
                memcpy(&cmd_buf[cmd_pos], data, copy);
                cmd_pos += copy;
            }
            break;

        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// Handle a register read from host
// ---------------------------------------------------------------------------
static void handle_reg_read(uint32_t reg_addr) {
    uint32_t val = 0;

    switch (reg_addr) {
        case TPM_REG_ACCESS:
            val = tpm_access_reg;
            spi_write_read_buffer(TPM_REG_ACCESS, &val, 1);
            break;

        case TPM_REG_STS: {
            uint32_t sts = tpm_sts_reg | TPM_STS_VALID;
            if (spi_state == kSpiResponding && resp_pos < resp_len) {
                sts |= TPM_STS_DATA_AVAIL;
            }
            val = sts;
            spi_write_read_buffer(TPM_REG_STS, &val, 1);
            break;
        }

        case TPM_REG_DATA_FIFO:
            // Host is reading response bytes — 4 bytes at a time
            if (spi_state == kSpiResponding && resp_pos < resp_len) {
                uint32_t remaining = resp_len - resp_pos;
                uint32_t chunk = (remaining >= 4) ? 4 : remaining;
                uint32_t word = 0;
                memcpy(&word, &resp_buf[resp_pos], chunk);
                spi_write_read_buffer(TPM_REG_DATA_FIFO, &word, 1);
                resp_pos += chunk;

                if (resp_pos >= resp_len) {
                    // Response fully read
                    tpm_sts_reg &= ~TPM_STS_DATA_AVAIL;
                    tpm_sts_reg |= TPM_STS_CMD_READY;
                    spi_state = kSpiIdle;
                }
            }
            break;

        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// tpm_spi_poll — Main SPI polling loop
// ---------------------------------------------------------------------------
void tpm_spi_poll(void) {
    uart_puts("[TPM-SPI] Entering poll loop\r\n");

    while (1) {
        // Check if SPI host sent a transaction
        if (!spi_cmd_available()) {
            continue;
        }

        uint8_t opcode = spi_read_cmd();
        uint32_t addr  = spi_read_addr();

        // SPI TPM transactions use a 1-byte opcode:
        //   bit 7: 0 = write, 1 = read
        //   bits 5:0: transfer size - 1
        bool is_read    = (opcode & 0x80U) != 0;
        uint32_t xfer_size = (opcode & 0x3FU) + 1;

        // Mask address to TPM register space (locality 0, 0x0000-0x0FFF)
        uint32_t reg_addr = addr & 0x0FFFU;

        if (is_read) {
            handle_reg_read(reg_addr);
        } else {
            // Read payload data from SPI ingress buffer
            uint32_t payload_words = (xfer_size + 3) / 4;
            uint32_t payload_buf[16];  // Max 64 bytes
            if (payload_words > 16) payload_words = 16;
            spi_read_payload(0, payload_buf, payload_words);
            handle_reg_write(reg_addr, (const uint8_t *)payload_buf, xfer_size);
        }

        // If command is complete, process it
        if (spi_state == kSpiProcessing) {
            resp_len = TPM_RESP_BUF_SIZE;
            tpm_process_command(cmd_buf, cmd_pos, resp_buf, &resp_len);

            // Set up for host to read response
            resp_pos = 0;
            spi_state = kSpiResponding;
            tpm_sts_reg = TPM_STS_VALID | TPM_STS_DATA_AVAIL;
        }
    }
}
