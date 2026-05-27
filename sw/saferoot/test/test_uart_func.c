// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// test_uart_func.c — Functional tests for UART peripheral
// Runs on Ibex CPU, drives UART via HAL + direct register access.
// Results output via UART.

#include "../hal/hal.h"
#include "../hal/uart_drv.h"

// -------------------------------------------------------------------------
// UART register offsets (from uart_drv.c / uart_reg_pkg.sv)
// -------------------------------------------------------------------------
#define UART_BASE        SAFEROOT_UART0_BASE
#define UART_CTRL        0x10
#define UART_STATUS      0x14
#define UART_RDATA       0x18
#define UART_WDATA       0x1C
#define UART_FIFO_CTRL   0x20
#define UART_FIFO_STATUS 0x24

// CTRL bits
#define UART_CTRL_TX_EN      BIT(0)
#define UART_CTRL_RX_EN      BIT(1)
#define UART_CTRL_NCO_SHIFT  16
#define UART_CTRL_NCO_MASK   0xFFFF0000U

// STATUS bits
#define UART_STATUS_TXFULL   BIT(0)
#define UART_STATUS_TXEMPTY  BIT(2)
#define UART_STATUS_TXIDLE   BIT(3)

// FIFO_CTRL bits
#define UART_FIFO_CTRL_RXRST BIT(0)
#define UART_FIFO_CTRL_TXRST BIT(1)

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

static void delay(volatile uint32_t n) {
    while (n--) { __asm__ volatile(""); }
}

// -------------------------------------------------------------------------
// Expected NCO value: baud / (clk >> 20)
// 115200 / (100000000 >> 20) = 115200 / 95 = 1212
// 9600 / 95 = 101
// -------------------------------------------------------------------------
#define NCO_115200  (115200U / (100000000U >> 20))  // 1212
#define NCO_9600    (9600U   / (100000000U >> 20))  // 101

// -------------------------------------------------------------------------
// Test 1: UART init — verify CTRL register
// -------------------------------------------------------------------------
static void test_uart_init(void) {
    hal_status_t st = uart_init(115200, 100000000);
    uint32_t ctrl = REG32_READ(UART_BASE + UART_CTRL);

    bool tx_en = (ctrl & UART_CTRL_TX_EN) != 0;
    uint32_t nco = (ctrl & UART_CTRL_NCO_MASK) >> UART_CTRL_NCO_SHIFT;
    bool nco_ok = (nco == NCO_115200);

    print_hex_line("CTRL = ", ctrl);
    print_hex_line("NCO  = ", nco);
    print_result("UART init (TX_EN + NCO=1212)", st == kHalOk && tx_en && nco_ok);
}

// -------------------------------------------------------------------------
// Test 2: TX FIFO — write 10 chars, verify TXEMPTY clears, then TXIDLE
// -------------------------------------------------------------------------
static void test_tx_fifo(void) {
    // Re-init clean state
    uart_init(115200, 100000000);

    // Write 10 characters quickly
    for (int i = 0; i < 10; i++) {
        REG32_WRITE(UART_BASE + UART_WDATA, (uint32_t)('0' + i));
    }

    // Immediately after burst write, TXEMPTY should be clear
    uint32_t st1 = REG32_READ(UART_BASE + UART_STATUS);
    bool not_empty = (st1 & UART_STATUS_TXEMPTY) == 0;

    // Wait for TX to drain
    delay(200000);
    uint32_t st2 = REG32_READ(UART_BASE + UART_STATUS);
    bool idle_after = (st2 & UART_STATUS_TXIDLE) != 0;

    print_hex_line("STATUS after write = ", st1);
    print_hex_line("STATUS after drain = ", st2);
    print_result("TX FIFO fill+drain", not_empty && idle_after);
}

// -------------------------------------------------------------------------
// Test 3: TX string — send string via uart_puts, verify STATUS
// -------------------------------------------------------------------------
static void test_tx_string(void) {
    uart_puts("Hello SafeRoot!");

    // After puts returns (blocking), FIFO may still be draining.
    // Wait a bit for TX to finish.
    delay(100000);
    uint32_t status = REG32_READ(UART_BASE + UART_STATUS);
    bool idle = (status & UART_STATUS_TXIDLE) != 0;

    uart_puts("\n");
    print_hex_line("STATUS = ", status);
    print_result("TX string 'Hello SafeRoot!'", idle);
}

// -------------------------------------------------------------------------
// Test 4: FIFO reset — write reset bits, verify FIFO_STATUS shows empty
// -------------------------------------------------------------------------
static void test_fifo_reset(void) {
    // Write some data first
    REG32_WRITE(UART_BASE + UART_WDATA, 0x41);
    REG32_WRITE(UART_BASE + UART_WDATA, 0x42);

    // Reset both FIFOs
    REG32_WRITE(UART_BASE + UART_FIFO_CTRL,
                UART_FIFO_CTRL_RXRST | UART_FIFO_CTRL_TXRST);

    delay(100);

    // Read FIFO_STATUS — after reset, TX and RX levels should be 0
    uint32_t fifo_st = REG32_READ(UART_BASE + UART_FIFO_STATUS);
    // FIFO_STATUS: [7:0] = TXLVL, [23:16] = RXLVL  (OpenTitan UART)
    uint32_t txlvl = fifo_st & 0xFF;
    uint32_t rxlvl = (fifo_st >> 16) & 0xFF;

    print_hex_line("FIFO_STATUS = ", fifo_st);
    print_result("FIFO reset (levels=0)", txlvl == 0 && rxlvl == 0);

    // Re-init UART after reset
    uart_init(115200, 100000000);
}

// -------------------------------------------------------------------------
// Test 5: Baud rate change — switch to 9600, verify NCO, switch back
// -------------------------------------------------------------------------
static void test_baud_rate(void) {
    // Switch to 9600 baud
    uart_init(9600, 100000000);
    uint32_t ctrl_9600 = REG32_READ(UART_BASE + UART_CTRL);
    uint32_t nco_9600 = (ctrl_9600 & UART_CTRL_NCO_MASK) >> UART_CTRL_NCO_SHIFT;
    bool nco_9600_ok = (nco_9600 == NCO_9600);

    // Switch back to 115200
    uart_init(115200, 100000000);
    uint32_t ctrl_115k = REG32_READ(UART_BASE + UART_CTRL);
    uint32_t nco_115k = (ctrl_115k & UART_CTRL_NCO_MASK) >> UART_CTRL_NCO_SHIFT;
    bool nco_115k_ok = (nco_115k == NCO_115200);

    print_hex_line("NCO @9600   = ", nco_9600);
    print_hex_line("NCO @115200 = ", nco_115k);
    print_result("Baud rate 9600->115200", nco_9600_ok && nco_115k_ok);
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
    uart_puts("  SafeRoot UART Functional Test\n");
    uart_puts("========================================\n");

    test_uart_init();
    test_tx_fifo();
    test_tx_string();
    test_fifo_reset();
    test_baud_rate();

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

    // Halt
    while (1) { __asm__ volatile("wfi"); }
}
