// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// gpio_drv.h — GPIO driver for SafeRoot (OpenTitan GPIO IP)

#ifndef SAFEROOT_GPIO_DRV_H_
#define SAFEROOT_GPIO_DRV_H_

#include "hal.h"

// Initialize GPIO (disable all outputs, clear all)
hal_status_t gpio_init(void);

// Read all 32 GPIO input pins
uint32_t gpio_read(void);

// Read a single pin (returns 0 or 1)
uint32_t gpio_read_bit(uint32_t pin);

// Write all 32 GPIO output pins
void gpio_write(uint32_t val);

// Set pin(s) high using masked write (no read-modify-write needed)
void gpio_set_bit(uint32_t pin);

// Clear pin(s) low using masked write
void gpio_clear_bit(uint32_t pin);

// Toggle pin(s) — requires read-modify-write
void gpio_toggle(uint32_t pin);

// Configure output enable for a pin (1 = output, 0 = input)
void gpio_set_output_en(uint32_t pin, bool enable);

#endif  // SAFEROOT_GPIO_DRV_H_
