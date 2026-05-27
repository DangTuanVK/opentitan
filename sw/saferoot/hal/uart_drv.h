// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// uart_drv.h — UART driver for SafeRoot (OpenTitan UART IP)

#ifndef SAFEROOT_UART_DRV_H_
#define SAFEROOT_UART_DRV_H_

#include "hal.h"

// Initialize UART with given baud rate (CPU clock in Hz)
hal_status_t uart_init(uint32_t baud_rate, uint32_t clk_freq_hz);

// Transmit a single character (blocking)
void uart_putc(char c);

// Transmit a null-terminated string
void uart_puts(const char *s);

// Receive a single character (blocking)
char uart_getc(void);

// Print a 32-bit value as 8-char hex string
void uart_put_hex(uint32_t val);

// Check if RX data is available (non-blocking)
bool uart_rx_available(void);

// Check if TX FIFO has space
bool uart_tx_ready(void);

#endif  // SAFEROOT_UART_DRV_H_
