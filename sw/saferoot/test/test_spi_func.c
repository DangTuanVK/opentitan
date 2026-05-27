// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// test_spi_func.c — Functional tests for SPI Device peripheral
// Runs on Ibex CPU, drives SPI Device via HAL + direct register access.
// Results output via UART.
//
// NOTE: SPI Device is slave-mode. Full functional test requires an
// external SPI master. These tests verify register access, FIFO
// operation, and JEDEC ID configuration.

#include "../hal/hal.h"
#include "../hal/uart_drv.h"
#include "../hal/spi_drv.h"

// -------------------------------------------------------------------------
// SPI Device register offsets (from spi_drv.c)
// -------------------------------------------------------------------------
#define SPI_BASE            SAFEROOT_SPI_DEVICE_BASE
#define SPI_INTR_STATE      0x00
#define SPI_CONTROL         0x10
#define SPI_CFG             0x14
#define SPI_STATUS          0x18
#define SPI_FLASH_STATUS    0x28
#define SPI_JEDEC_CC        0x2C
#define SPI_JEDEC_ID        0x30
#define SPI_UPLOAD_STATUS   0x3C
#define SPI_EGRESS_BUFFER   0x1000

// STATUS bits
#define SPI_STATUS_CSB      BIT(0)

// UPLOAD_STATUS fields
#define SPI_UPLOAD_CMDFIFO_DEPTH_MASK  0x1F

// -------------------------------------------------------------------------
// Test counters and helpers
// -------------------------------------------------------------------------
static uint32_t pass_count;
static uint32_t fail_count;

static void print_result(const char *name, bool ok) {
    uart_puts("  [TEST] ");
    uart_puts(name);
    uart_puts(": ");
    if (ok) {
        uart_puts("PASS\n");
        pass_count++;
    } else {
        uart_puts("FAIL\n");
        fail_count++;
    }
}

static void print_hex_line(const char *label, uint32_t val) {
    uart_puts("    ");
    uart_puts(label);
    uart_put_hex(val);
    uart_puts("\n");
}

// -------------------------------------------------------------------------
// Test 1: SPI init — verify CTRL register, STATUS accessible
// -------------------------------------------------------------------------
static void test_spi_init(void) {
    hal_status_t st = spi_init();

    uint32_t ctrl = REG32_READ(SPI_BASE + SPI_CONTROL);
    uint32_t status = REG32_READ(SPI_BASE + SPI_STATUS);

    print_hex_line("CONTROL = ", ctrl);
    print_hex_line("STATUS  = ", status);

    // After init: CONTROL should be 0 (flash mode, no abort)
    print_result("SPI init (CTRL=0)", st == kHalOk && ctrl == 0);
}

// -------------------------------------------------------------------------
// Test 2: TX FIFO write — write test words to egress buffer
// -------------------------------------------------------------------------
static void test_tx_fifo_write(void) {
    // Write 4 test words to the egress (read) buffer
    uint32_t test_data[4] = {0xDEADBEEF, 0xCAFEBABE, 0x12345678, 0xA5A5A5A5};
    hal_status_t st = spi_write_read_buffer(0, test_data, 4);

    // Read back from egress buffer directly
    bool match = true;
    for (uint32_t i = 0; i < 4; i++) {
        uint32_t rb = REG32_READ(SPI_BASE + SPI_EGRESS_BUFFER + (i * 4));
        if (rb != test_data[i]) {
            print_hex_line("Mismatch at word ", i);
            print_hex_line("  expected = ", test_data[i]);
            print_hex_line("  got      = ", rb);
            match = false;
        }
    }

    print_result("Egress buffer write+readback", st == kHalOk && match);
}

// -------------------------------------------------------------------------
// Test 3: Status register — read STATUS, verify accessible
// -------------------------------------------------------------------------
static void test_status_register(void) {
    uint32_t status = REG32_READ(SPI_BASE + SPI_STATUS);
    print_hex_line("STATUS = ", status);

    // CSB should be high (inactive) when no SPI master is connected
    // In simulation, CSB defaults to 1 (deasserted).
    bool csb_idle = (status & SPI_STATUS_CSB) != 0;

    print_result("STATUS reg (CSB idle)", csb_idle);
}

// -------------------------------------------------------------------------
// Test 4: JEDEC ID — configure and read back
// -------------------------------------------------------------------------
static void test_jedec_id(void) {
    // Set JEDEC ID: manufacturer=0xEF (Winbond-like), device_id=0x4018
    spi_set_jedec_id(0x4018, 0xEF, 0x00);

    uint32_t jedec_id = REG32_READ(SPI_BASE + SPI_JEDEC_ID);
    uint32_t jedec_cc = REG32_READ(SPI_BASE + SPI_JEDEC_CC);

    print_hex_line("JEDEC_ID = ", jedec_id);
    print_hex_line("JEDEC_CC = ", jedec_cc);

    // JEDEC_ID: [7:0]=manufacturer(0xEF), [23:8]=device_id(0x4018)
    uint8_t  mfr = (uint8_t)(jedec_id & 0xFF);
    uint16_t did = (uint16_t)((jedec_id >> 8) & 0xFFFF);

    bool id_ok = (mfr == 0xEF) && (did == 0x4018);
    print_result("JEDEC ID config+readback", id_ok);
}

// -------------------------------------------------------------------------
// Test 5: Loopback concept — write data, verify upload status empty
// -------------------------------------------------------------------------
static void test_loopback_concept(void) {
    // Without an external SPI master, the upload command FIFO should be
    // empty. Verify that UPLOAD_STATUS shows depth=0.
    uint32_t upload_st = REG32_READ(SPI_BASE + SPI_UPLOAD_STATUS);
    uint32_t cmd_depth = upload_st & SPI_UPLOAD_CMDFIFO_DEPTH_MASK;

    print_hex_line("UPLOAD_STATUS = ", upload_st);

    // Also test flash status register write+readback
    spi_set_flash_status(0x02);  // WEL bit
    uint32_t flash_st = REG32_READ(SPI_BASE + SPI_FLASH_STATUS);
    print_hex_line("FLASH_STATUS  = ", flash_st);

    bool fifo_empty = (cmd_depth == 0);
    bool flash_ok   = (flash_st & 0xFF) == 0x02;

    print_result("Upload FIFO empty + flash status", fifo_empty && flash_ok);
}

// -------------------------------------------------------------------------
// Main
// -------------------------------------------------------------------------
void main(void) {
    uart_init(115200, 100000000);
    pass_count = 0;
    fail_count = 0;

    uart_puts("\n");
    uart_puts("========================================\n");
    uart_puts("  SafeRoot SPI Device Functional Test\n");
    uart_puts("========================================\n");

    test_spi_init();
    test_tx_fifo_write();
    test_status_register();
    test_jedec_id();
    test_loopback_concept();

    uart_puts("\n--- Summary: ");
    uart_put_hex(pass_count);
    uart_puts(" PASS, ");
    uart_put_hex(fail_count);
    uart_puts(" FAIL ---\n");

    if (fail_count == 0) {
        uart_puts("ALL TESTS PASSED\n");
    } else {
        uart_puts("SOME TESTS FAILED\n");
    }

    while (1) { __asm__ volatile("wfi"); }
}
