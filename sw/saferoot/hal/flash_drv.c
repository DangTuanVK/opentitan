// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// flash_drv.c — Flash Controller driver for SafeRoot (OpenTitan FLASH_CTRL IP)
//
// The flash controller has two interfaces:
// 1. Memory-mapped read interface at FLASH_MEM_BASE (0x20000000)
// 2. Register interface at FLASH_CTRL_BASE (0x40170000) for program/erase
//
// Program/erase protocol:
//   1. Configure CONTROL register (op type, partition, word count, start addr)
//   2. Write data to PROG_FIFO (for program operations)
//   3. Trigger START
//   4. Poll STATUS for completion
//   5. Check for errors

#include "flash_drv.h"

// ---------------------------------------------------------------------------
// Flash Controller register offsets (OpenTitan standard layout)
// ---------------------------------------------------------------------------
#define FLASH_INTR_STATE       0x00
#define FLASH_INTR_ENABLE      0x04
#define FLASH_INTR_TEST        0x08
#define FLASH_ALERT_TEST       0x0C
#define FLASH_DIS              0x10  // Flash disable
#define FLASH_EXEC             0x14  // Flash execution enable
#define FLASH_INIT             0x18  // Flash controller init
#define FLASH_CTRL_REGWEN      0x1C  // Control register write enable
#define FLASH_CONTROL          0x20  // Control register
#define FLASH_ADDR             0x24  // Address register
#define FLASH_PROG_TYPE_EN     0x28  // Allowed program types
#define FLASH_ERASE_SUSPEND    0x2C  // Erase suspend
#define FLASH_REGION_CFG_REGWEN_0 0x30
// ... region config registers follow
#define FLASH_MP_BANK_CFG_SHADOWED 0x80
#define FLASH_OP_STATUS        0x84  // Operation status
#define FLASH_STATUS           0x88  // Controller status
// Error handling
#define FLASH_ERR_CODE         0x8C
#define FLASH_STD_FAULT_STATUS 0x90
#define FLASH_FAULT_STATUS     0x94
#define FLASH_ERR_ADDR         0x98
// FIFO access
#define FLASH_PROG_FIFO        0x9C  // Program data FIFO (WO)
#define FLASH_RD_FIFO          0xA0  // Read data FIFO (RO)

// ---------------------------------------------------------------------------
// CONTROL register bit definitions
// ---------------------------------------------------------------------------
// [0]     START — trigger operation
// [5:4]   OP — operation type: 0=read, 1=program, 2=erase
// [6]     PROG_SEL — 0=normal, 1=repair
// [7]     ERASE_SEL — 0=page erase, 1=bank erase
// [8]     PARTITION_SEL — 0=data, 1=info
// [9:10]  INFO_SEL — info partition index
// [27:16] NUM — number of words minus 1
#define FLASH_CTRL_START       BIT(0)
#define FLASH_CTRL_OP_SHIFT    4
#define FLASH_CTRL_OP_MASK     0x3
#define FLASH_CTRL_PROG_SEL    BIT(6)
#define FLASH_CTRL_ERASE_SEL   BIT(7)
#define FLASH_CTRL_PART_SEL    BIT(8)
#define FLASH_CTRL_NUM_SHIFT   16

// ---------------------------------------------------------------------------
// STATUS register bit definitions
// ---------------------------------------------------------------------------
// [0] RD_FULL  — read FIFO full
// [1] RD_EMPTY — read FIFO empty
// [2] PROG_FULL  — program FIFO full
// [3] PROG_EMPTY — program FIFO empty
// [4] INIT_WIP — initialization in progress
#define FLASH_STATUS_RD_FULL    BIT(0)
#define FLASH_STATUS_RD_EMPTY   BIT(1)
#define FLASH_STATUS_PROG_FULL  BIT(2)
#define FLASH_STATUS_PROG_EMPTY BIT(3)
#define FLASH_STATUS_INIT_WIP   BIT(4)

// ---------------------------------------------------------------------------
// OP_STATUS register bit definitions
// ---------------------------------------------------------------------------
// [0] DONE — operation complete
// [1] ERR  — operation error
#define FLASH_OP_STATUS_DONE   BIT(0)
#define FLASH_OP_STATUS_ERR    BIT(1)

// ---------------------------------------------------------------------------
// INTR_STATE bits
// ---------------------------------------------------------------------------
#define FLASH_INTR_PROG_EMPTY  BIT(0)
#define FLASH_INTR_PROG_LVL    BIT(1)
#define FLASH_INTR_RD_FULL     BIT(2)
#define FLASH_INTR_RD_LVL      BIT(3)
#define FLASH_INTR_OP_DONE     BIT(4)
#define FLASH_INTR_CORR_ERR    BIT(5)

#define FLASH_BASE SAFEROOT_FLASH_CTRL_BASE
#define FLASH_MEM  SAFEROOT_FLASH_MEM_BASE

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static hal_status_t flash_wait_op_done(void) {
    for (uint32_t i = 0; i < HAL_TIMEOUT_CYCLES; i++) {
        uint32_t op_st = REG32_READ(FLASH_BASE + FLASH_OP_STATUS);
        if (op_st & FLASH_OP_STATUS_ERR) {
            return kHalError;
        }
        if (op_st & FLASH_OP_STATUS_DONE) {
            // Clear done status by writing 1
            REG32_WRITE(FLASH_BASE + FLASH_OP_STATUS, FLASH_OP_STATUS_DONE);
            return kHalOk;
        }
    }
    return kHalTimeout;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool flash_is_idle(void) {
    uint32_t st = REG32_READ(FLASH_BASE + FLASH_STATUS);
    return !(st & FLASH_STATUS_INIT_WIP);
}

hal_status_t flash_init(void) {
    // Clear pending interrupts
    REG32_WRITE(FLASH_BASE + FLASH_INTR_STATE, 0xFFFFFFFF);
    // Disable interrupts
    REG32_WRITE(FLASH_BASE + FLASH_INTR_ENABLE, 0);
    // Trigger controller init
    REG32_WRITE(FLASH_BASE + FLASH_INIT, 1);
    // Wait for init to complete
    for (uint32_t i = 0; i < HAL_TIMEOUT_CYCLES; i++) {
        if (!(REG32_READ(FLASH_BASE + FLASH_STATUS) & FLASH_STATUS_INIT_WIP)) {
            return kHalOk;
        }
    }
    return kHalTimeout;
}

hal_status_t flash_read(uint32_t addr, uint32_t *data, uint32_t word_count) {
    if (data == (void *)0 || word_count == 0) return kHalBadArg;
    // Direct memory-mapped read — simplest and fastest method
    const volatile uint32_t *src = (const volatile uint32_t *)(FLASH_MEM + addr);
    for (uint32_t i = 0; i < word_count; i++) {
        data[i] = src[i];
    }
    return kHalOk;
}

hal_status_t flash_write(uint32_t addr, const uint32_t *data,
                         uint32_t word_count) {
    if (data == (void *)0 || word_count == 0) return kHalBadArg;
    if (addr & 0x3) return kHalBadArg;

    // Set address
    REG32_WRITE(FLASH_BASE + FLASH_ADDR, addr);

    // Configure: program operation, data partition, word count
    uint32_t ctrl = ((uint32_t)kFlashOpProg << FLASH_CTRL_OP_SHIFT) |
                    ((word_count - 1) << FLASH_CTRL_NUM_SHIFT) |
                    FLASH_CTRL_START;
    REG32_WRITE(FLASH_BASE + FLASH_CONTROL, ctrl);

    // Write data to program FIFO
    for (uint32_t i = 0; i < word_count; i++) {
        // Wait for FIFO space
        while (REG32_READ(FLASH_BASE + FLASH_STATUS) & FLASH_STATUS_PROG_FULL) {
            // spin
        }
        REG32_WRITE(FLASH_BASE + FLASH_PROG_FIFO, data[i]);
    }

    // Wait for operation to complete
    return flash_wait_op_done();
}

hal_status_t flash_erase_page(uint32_t addr) {
    // Set address (any address within the page)
    REG32_WRITE(FLASH_BASE + FLASH_ADDR, addr);

    // Configure: erase operation, page erase, data partition
    uint32_t ctrl = ((uint32_t)kFlashOpErase << FLASH_CTRL_OP_SHIFT) |
                    FLASH_CTRL_START;
    // ERASE_SEL = 0 means page erase (not bank erase)
    REG32_WRITE(FLASH_BASE + FLASH_CONTROL, ctrl);

    // Wait for operation to complete
    return flash_wait_op_done();
}
