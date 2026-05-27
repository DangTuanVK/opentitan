// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// flash_drv.h — Flash Controller driver for SafeRoot (OpenTitan FLASH_CTRL IP)
// Supports read, write (program), and erase operations.

#ifndef SAFEROOT_FLASH_DRV_H_
#define SAFEROOT_FLASH_DRV_H_

#include "hal.h"

// Flash geometry (typical OpenTitan values)
#define FLASH_PAGE_SIZE      2048U  // 2 KB per page (in bytes)
#define FLASH_WORDS_PER_PAGE (FLASH_PAGE_SIZE / 4)
#define FLASH_BANK_PAGES     256U
#define FLASH_NUM_BANKS      2U

// Flash operation types
typedef enum {
    kFlashOpRead  = 0,
    kFlashOpProg  = 1,  // Program (write)
    kFlashOpErase = 2,
} flash_op_t;

// Flash erase type
typedef enum {
    kFlashErasePage = 0,
    kFlashEraseBank = 1,
} flash_erase_t;

// Initialize flash controller
hal_status_t flash_init(void);

// Read words from flash via memory-mapped interface (fastest).
// addr: byte address in flash memory space.
// data: destination buffer.
// word_count: number of 32-bit words to read.
hal_status_t flash_read(uint32_t addr, uint32_t *data, uint32_t word_count);

// Program (write) words to flash via the controller.
// addr: byte address in flash (must be word-aligned).
// data: source buffer.
// word_count: number of 32-bit words to program.
hal_status_t flash_write(uint32_t addr, const uint32_t *data,
                         uint32_t word_count);

// Erase a flash page.
// addr: any byte address within the page to erase.
hal_status_t flash_erase_page(uint32_t addr);

// Check if flash controller is idle
bool flash_is_idle(void);

#endif  // SAFEROOT_FLASH_DRV_H_
