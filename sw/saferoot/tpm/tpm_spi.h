// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// tpm_spi.h — TPM over SPI transport layer for SafeRoot
// Implements TCG PC Client Platform TPM Profile (PTP) SPI interface.

#ifndef SAFEROOT_TPM_SPI_H_
#define SAFEROOT_TPM_SPI_H_

#include "tpm.h"

// ---------------------------------------------------------------------------
// TPM SPI Register Offsets (TCG PTP Spec, Table 19)
// ---------------------------------------------------------------------------
#define TPM_REG_ACCESS        0x0000U
#define TPM_REG_STS           0x0018U
#define TPM_REG_DATA_FIFO     0x0024U
#define TPM_REG_INTF_CAP      0x0014U
#define TPM_REG_INT_ENABLE    0x0008U
#define TPM_REG_INT_VECTOR    0x000CU
#define TPM_REG_INT_STATUS    0x0010U
#define TPM_REG_DID_VID       0x0F00U
#define TPM_REG_RID           0x0F04U

// ---------------------------------------------------------------------------
// TPM_ACCESS register bits
// ---------------------------------------------------------------------------
#define TPM_ACCESS_VALID       (1U << 7)
#define TPM_ACCESS_ACTIVE      (1U << 5)
#define TPM_ACCESS_BEEN_SEIZED (1U << 4)
#define TPM_ACCESS_SEIZE       (1U << 3)
#define TPM_ACCESS_PENDING     (1U << 2)
#define TPM_ACCESS_REQUEST     (1U << 1)
#define TPM_ACCESS_ESTABLISH   (1U << 0)

// ---------------------------------------------------------------------------
// TPM_STS register bits
// ---------------------------------------------------------------------------
#define TPM_STS_VALID          (1U << 7)
#define TPM_STS_CMD_READY      (1U << 6)
#define TPM_STS_GO             (1U << 5)
#define TPM_STS_DATA_AVAIL     (1U << 4)
#define TPM_STS_EXPECT         (1U << 3)
#define TPM_STS_SELFTEST_DONE  (1U << 2)
#define TPM_STS_RESP_RETRY     (1U << 1)

// ---------------------------------------------------------------------------
// SPI Transport Constants
// ---------------------------------------------------------------------------
#define TPM_SPI_MAX_TRANSFER   4096U
#define TPM_CMD_BUF_SIZE       4096U
#define TPM_RESP_BUF_SIZE      4096U

// ---------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------

// Initialize TPM SPI transport layer
void tpm_spi_init(void);

// Poll for SPI transactions and process TPM commands.
// This is the main loop entry point — call from firmware main().
// Returns only on fatal error.
void tpm_spi_poll(void);

#endif  // SAFEROOT_TPM_SPI_H_
