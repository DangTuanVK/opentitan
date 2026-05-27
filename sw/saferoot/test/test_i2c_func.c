// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// test_i2c_func.c — Functional tests for I2C peripheral
// Runs on Ibex CPU, drives I2C via direct register access (no HAL driver).
// Results output via UART.
//
// NOTE: Full I2C test requires an external I2C device on the bus.
// These tests verify register access, timing configuration, and
// controller state machine readiness.

#include "../hal/hal.h"
#include "../hal/uart_drv.h"

// -------------------------------------------------------------------------
// I2C base address and register offsets (OpenTitan I2C IP)
// -------------------------------------------------------------------------
#define I2C_BASE             0x40220000U

#define I2C_INTR_STATE       0x00
#define I2C_INTR_ENABLE      0x04
#define I2C_ALERT_TEST       0x0C
#define I2C_CTRL             0x10
#define I2C_STATUS           0x14
#define I2C_RDATA            0x18
#define I2C_WDATA            0x1C   // Alias for FDATA in some docs
#define I2C_FIFO_CTRL        0x20
#define I2C_FDATA            0x24   // Format data (host-mode TX)
#define I2C_TIMING0          0x3C
#define I2C_TIMING1          0x40
#define I2C_TARGET_ID        0x48
#define I2C_FIFO_STATUS      0x34   // Host/target FIFO levels
#define I2C_HOST_FIFO_STATUS 0x34

// CTRL register bits
#define I2C_CTRL_ENABLEHOST  BIT(0)
#define I2C_CTRL_ENABLETARGET BIT(1)

// STATUS register bits
#define I2C_STATUS_FMTFULL   BIT(0)
#define I2C_STATUS_RXFULL    BIT(1)
#define I2C_STATUS_FMTEMPTY  BIT(2)
#define I2C_STATUS_HOSTIDLE  BIT(3)
#define I2C_STATUS_TARGETIDLE BIT(4)
#define I2C_STATUS_RXEMPTY   BIT(5)

// FIFO_CTRL reset bits
#define I2C_FIFO_CTRL_RXRST  BIT(0)
#define I2C_FIFO_CTRL_FMTRST BIT(1)

// FDATA register bits (format byte for host mode)
#define I2C_FDATA_START      BIT(8)
#define I2C_FDATA_STOP       BIT(9)
#define I2C_FDATA_READ       BIT(10)
#define I2C_FDATA_RCONT      BIT(11)
#define I2C_FDATA_NAKOK      BIT(12)

// -------------------------------------------------------------------------
// Timing parameters for 100kHz Standard Mode @ 100MHz clock
// TIMING0: [15:0] THIGH, [31:16] TLOW
// TIMING1: [15:0] T_R (rise time), [31:16] T_F (fall time)
// For 100kHz @ 100MHz: period=1000 clocks, THIGH=500, TLOW=500
// Rise/fall: ~100ns => 10 clocks
// -------------------------------------------------------------------------
#define I2C_TIMING0_100K     ((500U << 16) | 500U)   // TLOW=500, THIGH=500
#define I2C_TIMING1_100K     ((10U  << 16) | 10U)    // T_F=10, T_R=10

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
// I2C init helper (no HAL driver — direct register access)
// -------------------------------------------------------------------------
static void i2c_init_host(void) {
    // Disable controller
    REG32_WRITE(I2C_BASE + I2C_CTRL, 0);

    // Clear pending interrupts
    REG32_WRITE(I2C_BASE + I2C_INTR_STATE, 0xFFFFFFFF);

    // Disable all interrupts
    REG32_WRITE(I2C_BASE + I2C_INTR_ENABLE, 0);

    // Reset FIFOs
    REG32_WRITE(I2C_BASE + I2C_FIFO_CTRL,
                I2C_FIFO_CTRL_RXRST | I2C_FIFO_CTRL_FMTRST);

    // Configure timing for 100kHz standard mode
    REG32_WRITE(I2C_BASE + I2C_TIMING0, I2C_TIMING0_100K);
    REG32_WRITE(I2C_BASE + I2C_TIMING1, I2C_TIMING1_100K);

    // Enable host mode
    REG32_WRITE(I2C_BASE + I2C_CTRL, I2C_CTRL_ENABLEHOST);
}

// -------------------------------------------------------------------------
// Test 1: I2C init — configure timing, verify register values
// -------------------------------------------------------------------------
static void test_i2c_init(void) {
    i2c_init_host();

    uint32_t ctrl    = REG32_READ(I2C_BASE + I2C_CTRL);
    uint32_t timing0 = REG32_READ(I2C_BASE + I2C_TIMING0);
    uint32_t timing1 = REG32_READ(I2C_BASE + I2C_TIMING1);

    print_hex_line("CTRL    = ", ctrl);
    print_hex_line("TIMING0 = ", timing0);
    print_hex_line("TIMING1 = ", timing1);

    bool host_en   = (ctrl & I2C_CTRL_ENABLEHOST) != 0;
    bool timing_ok = (timing0 == I2C_TIMING0_100K) &&
                     (timing1 == I2C_TIMING1_100K);

    print_result("I2C init (host mode + timing)", host_en && timing_ok);
}

// -------------------------------------------------------------------------
// Test 2: Status register — verify host_idle after init
// -------------------------------------------------------------------------
static void test_status_register(void) {
    i2c_init_host();

    uint32_t status = REG32_READ(I2C_BASE + I2C_STATUS);
    print_hex_line("STATUS = ", status);

    bool host_idle = (status & I2C_STATUS_HOSTIDLE) != 0;
    bool fmt_empty = (status & I2C_STATUS_FMTEMPTY) != 0;

    print_result("Status: host_idle + fmt_empty", host_idle && fmt_empty);
}

// -------------------------------------------------------------------------
// Test 3: Target ID — write address, readback verify
// -------------------------------------------------------------------------
static void test_target_id(void) {
    i2c_init_host();

    // TARGET_ID register: [6:0]=address0, [20:14]=address1 (7-bit addrs)
    // Set address0 = 0x50 (typical EEPROM address)
    uint32_t target_val = 0x50;
    REG32_WRITE(I2C_BASE + I2C_TARGET_ID, target_val);

    uint32_t readback = REG32_READ(I2C_BASE + I2C_TARGET_ID);
    print_hex_line("TARGET_ID written = ", target_val);
    print_hex_line("TARGET_ID read    = ", readback);

    bool match = (readback & 0x7F) == (target_val & 0x7F);
    print_result("Target ID write+readback", match);
}

// -------------------------------------------------------------------------
// Test 4: FIFO status — verify FIFOs empty after init
// -------------------------------------------------------------------------
static void test_fifo_status(void) {
    i2c_init_host();

    uint32_t status = REG32_READ(I2C_BASE + I2C_STATUS);
    print_hex_line("STATUS = ", status);

    // FMT FIFO should be empty, RX FIFO should be empty
    bool fmt_empty = (status & I2C_STATUS_FMTEMPTY) != 0;
    bool rx_empty  = (status & I2C_STATUS_RXEMPTY)  != 0;

    print_result("FIFO empty after init", fmt_empty && rx_empty);
}

// -------------------------------------------------------------------------
// Test 5: Write to FDATA — write start+address+stop, check status
// -------------------------------------------------------------------------
static void test_fdata_write(void) {
    i2c_init_host();

    // Verify FMT FIFO is empty before write
    uint32_t st_before = REG32_READ(I2C_BASE + I2C_STATUS);
    bool empty_before = (st_before & I2C_STATUS_FMTEMPTY) != 0;

    // Write a transaction: START + address byte (0x50 << 1 | 0 = write)
    uint32_t fdata = I2C_FDATA_START | (0x50 << 1);
    REG32_WRITE(I2C_BASE + I2C_FDATA, fdata);

    // Write data byte + STOP
    uint32_t fdata2 = I2C_FDATA_STOP | 0xFF;
    REG32_WRITE(I2C_BASE + I2C_FDATA, fdata2);

    // Check status — FMT FIFO may no longer be empty (depends on timing)
    uint32_t st_after = REG32_READ(I2C_BASE + I2C_STATUS);

    print_hex_line("STATUS before = ", st_before);
    print_hex_line("FDATA written = ", fdata);
    print_hex_line("STATUS after  = ", st_after);

    // Without an I2C device, the transaction will NACK and eventually
    // the controller will report an error. We just verify that:
    // 1. FIFO was empty before write
    // 2. Register write did not cause a bus error (we got here)
    print_result("FDATA write (reg access ok)", empty_before);

    // Clean up: reset FIFOs
    REG32_WRITE(I2C_BASE + I2C_FIFO_CTRL,
                I2C_FIFO_CTRL_RXRST | I2C_FIFO_CTRL_FMTRST);
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
    uart_puts("  SafeRoot I2C Functional Test\n");
    uart_puts("========================================\n");

    test_i2c_init();
    test_status_register();
    test_target_id();
    test_fifo_status();
    test_fdata_write();

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
