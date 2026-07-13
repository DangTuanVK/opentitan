// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// ------------------- W A R N I N G: A U T O - G E N E R A T E D   C O D E !! -------------------//
// PLEASE DO NOT HAND-EDIT THIS FILE. IT HAS BEEN AUTO-GENERATED WITH THE FOLLOWING COMMAND:
// util/topgen.py -t hw/top_saferoot/data/top_saferoot.hjson
//                -o hw/top_saferoot/

#ifndef OPENTITAN_HW_TOP_SAFEROOT_SW_AUTOGEN_TOP_SAFEROOT_H_
#define OPENTITAN_HW_TOP_SAFEROOT_SW_AUTOGEN_TOP_SAFEROOT_H_

/**
 * @file
 * @brief Top-specific Definitions
 *
 * This file contains preprocessor and type definitions for use within the
 * device C/C++ codebase.
 *
 * These definitions are for information that depends on the top-specific chip
 * configuration, which includes:
 * - Device Memory Information (for Peripherals and Memory)
 * - PLIC Interrupt ID Names and Source Mappings
 * - Alert ID Names and Source Mappings
 * - Pinmux Pin/Select Names
 * - Power Manager Wakeups
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Peripheral base address for aes in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_AES_BASE_ADDR 0x40000000u

/**
 * Peripheral size for aes in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_AES_BASE_ADDR and
 * `TOP_SAFEROOT_AES_BASE_ADDR + TOP_SAFEROOT_AES_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_AES_SIZE_BYTES 0x100u

/**
 * Peripheral base address for hmac in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_HMAC_BASE_ADDR 0x40010000u

/**
 * Peripheral size for hmac in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_HMAC_BASE_ADDR and
 * `TOP_SAFEROOT_HMAC_BASE_ADDR + TOP_SAFEROOT_HMAC_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_HMAC_SIZE_BYTES 0x2000u

/**
 * Peripheral base address for kmac in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_KMAC_BASE_ADDR 0x40020000u

/**
 * Peripheral size for kmac in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_KMAC_BASE_ADDR and
 * `TOP_SAFEROOT_KMAC_BASE_ADDR + TOP_SAFEROOT_KMAC_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_KMAC_SIZE_BYTES 0x1000u

/**
 * Peripheral base address for csrng in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_CSRNG_BASE_ADDR 0x40030000u

/**
 * Peripheral size for csrng in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_CSRNG_BASE_ADDR and
 * `TOP_SAFEROOT_CSRNG_BASE_ADDR + TOP_SAFEROOT_CSRNG_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_CSRNG_SIZE_BYTES 0x80u

/**
 * Peripheral base address for entropy_src in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_ENTROPY_SRC_BASE_ADDR 0x40040000u

/**
 * Peripheral size for entropy_src in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_ENTROPY_SRC_BASE_ADDR and
 * `TOP_SAFEROOT_ENTROPY_SRC_BASE_ADDR + TOP_SAFEROOT_ENTROPY_SRC_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_ENTROPY_SRC_SIZE_BYTES 0x100u

/**
 * Peripheral base address for edn0 in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_EDN0_BASE_ADDR 0x40005000u

/**
 * Peripheral size for edn0 in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_EDN0_BASE_ADDR and
 * `TOP_SAFEROOT_EDN0_BASE_ADDR + TOP_SAFEROOT_EDN0_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_EDN0_SIZE_BYTES 0x80u

/**
 * Peripheral base address for otbn in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_OTBN_BASE_ADDR 0x40080000u

/**
 * Peripheral size for otbn in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_OTBN_BASE_ADDR and
 * `TOP_SAFEROOT_OTBN_BASE_ADDR + TOP_SAFEROOT_OTBN_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_OTBN_SIZE_BYTES 0x10000u

/**
 * Peripheral base address for keymgr in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_KEYMGR_BASE_ADDR 0x40100000u

/**
 * Peripheral size for keymgr in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_KEYMGR_BASE_ADDR and
 * `TOP_SAFEROOT_KEYMGR_BASE_ADDR + TOP_SAFEROOT_KEYMGR_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_KEYMGR_SIZE_BYTES 0x100u

/**
 * Peripheral base address for regs device on lc_ctrl in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_LC_CTRL_REGS_BASE_ADDR 0x40101000u

/**
 * Peripheral size for regs device on lc_ctrl in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_LC_CTRL_REGS_BASE_ADDR and
 * `TOP_SAFEROOT_LC_CTRL_REGS_BASE_ADDR + TOP_SAFEROOT_LC_CTRL_REGS_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_LC_CTRL_REGS_SIZE_BYTES 0x100u

/**
 * Peripheral base address for dmi device on lc_ctrl in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_LC_CTRL_DMI_BASE_ADDR 0x0u

/**
 * Peripheral size for dmi device on lc_ctrl in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_LC_CTRL_DMI_BASE_ADDR and
 * `TOP_SAFEROOT_LC_CTRL_DMI_BASE_ADDR + TOP_SAFEROOT_LC_CTRL_DMI_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_LC_CTRL_DMI_SIZE_BYTES 0x1000u

/**
 * Peripheral base address for alert_handler in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_ALERT_HANDLER_BASE_ADDR 0x40120000u

/**
 * Peripheral size for alert_handler in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_ALERT_HANDLER_BASE_ADDR and
 * `TOP_SAFEROOT_ALERT_HANDLER_BASE_ADDR + TOP_SAFEROOT_ALERT_HANDLER_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_ALERT_HANDLER_SIZE_BYTES 0x800u

/**
 * Peripheral base address for core device on otp_ctrl in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_OTP_CTRL_CORE_BASE_ADDR 0x40130000u

/**
 * Peripheral size for core device on otp_ctrl in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_OTP_CTRL_CORE_BASE_ADDR and
 * `TOP_SAFEROOT_OTP_CTRL_CORE_BASE_ADDR + TOP_SAFEROOT_OTP_CTRL_CORE_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_OTP_CTRL_CORE_SIZE_BYTES 0x1000u

/**
 * Peripheral base address for prim device on otp_macro in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_OTP_MACRO_PRIM_BASE_ADDR 0x40138000u

/**
 * Peripheral size for prim device on otp_macro in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_OTP_MACRO_PRIM_BASE_ADDR and
 * `TOP_SAFEROOT_OTP_MACRO_PRIM_BASE_ADDR + TOP_SAFEROOT_OTP_MACRO_PRIM_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_OTP_MACRO_PRIM_SIZE_BYTES 0x20u

/**
 * Peripheral base address for sensor_ctrl in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_SENSOR_CTRL_BASE_ADDR 0x40140000u

/**
 * Peripheral size for sensor_ctrl in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_SENSOR_CTRL_BASE_ADDR and
 * `TOP_SAFEROOT_SENSOR_CTRL_BASE_ADDR + TOP_SAFEROOT_SENSOR_CTRL_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_SENSOR_CTRL_SIZE_BYTES 0x80u

/**
 * Peripheral base address for spi_device in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_SPI_DEVICE_BASE_ADDR 0x40200000u

/**
 * Peripheral size for spi_device in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_SPI_DEVICE_BASE_ADDR and
 * `TOP_SAFEROOT_SPI_DEVICE_BASE_ADDR + TOP_SAFEROOT_SPI_DEVICE_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_SPI_DEVICE_SIZE_BYTES 0x2000u

/**
 * Peripheral base address for gpio in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_GPIO_BASE_ADDR 0x40210000u

/**
 * Peripheral size for gpio in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_GPIO_BASE_ADDR and
 * `TOP_SAFEROOT_GPIO_BASE_ADDR + TOP_SAFEROOT_GPIO_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_GPIO_SIZE_BYTES 0x80u

/**
 * Peripheral base address for i2c0 in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_I2C0_BASE_ADDR 0x40220000u

/**
 * Peripheral size for i2c0 in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_I2C0_BASE_ADDR and
 * `TOP_SAFEROOT_I2C0_BASE_ADDR + TOP_SAFEROOT_I2C0_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_I2C0_SIZE_BYTES 0x80u

/**
 * Peripheral base address for uart0 in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_UART0_BASE_ADDR 0x40230000u

/**
 * Peripheral size for uart0 in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_UART0_BASE_ADDR and
 * `TOP_SAFEROOT_UART0_BASE_ADDR + TOP_SAFEROOT_UART0_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_UART0_SIZE_BYTES 0x40u

/**
 * Peripheral base address for rv_plic in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_RV_PLIC_BASE_ADDR 0x48000000u

/**
 * Peripheral size for rv_plic in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_RV_PLIC_BASE_ADDR and
 * `TOP_SAFEROOT_RV_PLIC_BASE_ADDR + TOP_SAFEROOT_RV_PLIC_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_RV_PLIC_SIZE_BYTES 0x8000000u

/**
 * Peripheral base address for rv_timer in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_RV_TIMER_BASE_ADDR 0x40310000u

/**
 * Peripheral size for rv_timer in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_RV_TIMER_BASE_ADDR and
 * `TOP_SAFEROOT_RV_TIMER_BASE_ADDR + TOP_SAFEROOT_RV_TIMER_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_RV_TIMER_SIZE_BYTES 0x200u

/**
 * Peripheral base address for aon_timer_aon in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_AON_TIMER_AON_BASE_ADDR 0x40320000u

/**
 * Peripheral size for aon_timer_aon in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_AON_TIMER_AON_BASE_ADDR and
 * `TOP_SAFEROOT_AON_TIMER_AON_BASE_ADDR + TOP_SAFEROOT_AON_TIMER_AON_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_AON_TIMER_AON_SIZE_BYTES 0x40u

/**
 * Peripheral base address for pwrmgr_aon in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_PWRMGR_AON_BASE_ADDR 0x40330000u

/**
 * Peripheral size for pwrmgr_aon in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_PWRMGR_AON_BASE_ADDR and
 * `TOP_SAFEROOT_PWRMGR_AON_BASE_ADDR + TOP_SAFEROOT_PWRMGR_AON_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_PWRMGR_AON_SIZE_BYTES 0x80u

/**
 * Peripheral base address for clkmgr_aon in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_CLKMGR_AON_BASE_ADDR 0x40340000u

/**
 * Peripheral size for clkmgr_aon in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_CLKMGR_AON_BASE_ADDR and
 * `TOP_SAFEROOT_CLKMGR_AON_BASE_ADDR + TOP_SAFEROOT_CLKMGR_AON_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_CLKMGR_AON_SIZE_BYTES 0x40u

/**
 * Peripheral base address for rstmgr_aon in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_RSTMGR_AON_BASE_ADDR 0x40350000u

/**
 * Peripheral size for rstmgr_aon in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_RSTMGR_AON_BASE_ADDR and
 * `TOP_SAFEROOT_RSTMGR_AON_BASE_ADDR + TOP_SAFEROOT_RSTMGR_AON_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_RSTMGR_AON_SIZE_BYTES 0x40u

/**
 * Peripheral base address for pinmux_aon in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_PINMUX_AON_BASE_ADDR 0x40360000u

/**
 * Peripheral size for pinmux_aon in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_PINMUX_AON_BASE_ADDR and
 * `TOP_SAFEROOT_PINMUX_AON_BASE_ADDR + TOP_SAFEROOT_PINMUX_AON_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_PINMUX_AON_SIZE_BYTES 0x400u

/**
 * Peripheral base address for regs device on rom_ctrl in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_ROM_CTRL_REGS_BASE_ADDR 0x40007000u

/**
 * Peripheral size for regs device on rom_ctrl in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_ROM_CTRL_REGS_BASE_ADDR and
 * `TOP_SAFEROOT_ROM_CTRL_REGS_BASE_ADDR + TOP_SAFEROOT_ROM_CTRL_REGS_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_ROM_CTRL_REGS_SIZE_BYTES 0x80u

/**
 * Peripheral base address for regs device on sram_ctrl in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_SRAM_CTRL_REGS_BASE_ADDR 0x40008000u

/**
 * Peripheral size for regs device on sram_ctrl in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_SRAM_CTRL_REGS_BASE_ADDR and
 * `TOP_SAFEROOT_SRAM_CTRL_REGS_BASE_ADDR + TOP_SAFEROOT_SRAM_CTRL_REGS_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_SRAM_CTRL_REGS_SIZE_BYTES 0x40u

/**
 * Peripheral base address for core device on flash_ctrl in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_FLASH_CTRL_CORE_BASE_ADDR 0x40009000u

/**
 * Peripheral size for core device on flash_ctrl in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_FLASH_CTRL_CORE_BASE_ADDR and
 * `TOP_SAFEROOT_FLASH_CTRL_CORE_BASE_ADDR + TOP_SAFEROOT_FLASH_CTRL_CORE_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_FLASH_CTRL_CORE_SIZE_BYTES 0x200u

/**
 * Peripheral base address for prim device on flash_ctrl in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_FLASH_CTRL_PRIM_BASE_ADDR 0x4000A000u

/**
 * Peripheral size for prim device on flash_ctrl in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_FLASH_CTRL_PRIM_BASE_ADDR and
 * `TOP_SAFEROOT_FLASH_CTRL_PRIM_BASE_ADDR + TOP_SAFEROOT_FLASH_CTRL_PRIM_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_FLASH_CTRL_PRIM_SIZE_BYTES 0x80u

/**
 * Peripheral base address for cfg device on rv_core_ibex in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_RV_CORE_IBEX_CFG_BASE_ADDR 0x4000B000u

/**
 * Peripheral size for cfg device on rv_core_ibex in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_RV_CORE_IBEX_CFG_BASE_ADDR and
 * `TOP_SAFEROOT_RV_CORE_IBEX_CFG_BASE_ADDR + TOP_SAFEROOT_RV_CORE_IBEX_CFG_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_RV_CORE_IBEX_CFG_SIZE_BYTES 0x100u


/**
 * Memory base address for rom memory on rom_ctrl in top saferoot.
 */
#define TOP_SAFEROOT_ROM_CTRL_ROM_BASE_ADDR 0x0u

/**
 * Memory size for rom memory on rom_ctrl in top saferoot.
 */
#define TOP_SAFEROOT_ROM_CTRL_ROM_SIZE_BYTES 0x8000u

/**
 * Memory base address for ram memory on sram_ctrl in top saferoot.
 */
#define TOP_SAFEROOT_SRAM_CTRL_RAM_BASE_ADDR 0x10000000u

/**
 * Memory size for ram memory on sram_ctrl in top saferoot.
 */
#define TOP_SAFEROOT_SRAM_CTRL_RAM_SIZE_BYTES 0x20000u

/**
 * Memory base address for mem memory on flash_ctrl in top saferoot.
 */
#define TOP_SAFEROOT_FLASH_CTRL_MEM_BASE_ADDR 0x20000000u

/**
 * Memory size for mem memory on flash_ctrl in top saferoot.
 */
#define TOP_SAFEROOT_FLASH_CTRL_MEM_SIZE_BYTES 0x100000u


/**
 * PLIC Interrupt Source Peripheral.
 *
 * Enumeration used to determine which peripheral asserted the corresponding
 * interrupt.
 */
typedef enum top_saferoot_plic_peripheral {
  kTopSaferootPlicPeripheralUnknown = 0, /**< Unknown Peripheral */
  kTopSaferootPlicPeripheralHmac = 1, /**< hmac */
  kTopSaferootPlicPeripheralKmac = 2, /**< kmac */
  kTopSaferootPlicPeripheralCsrng = 3, /**< csrng */
  kTopSaferootPlicPeripheralEntropySrc = 4, /**< entropy_src */
  kTopSaferootPlicPeripheralEdn0 = 5, /**< edn0 */
  kTopSaferootPlicPeripheralOtbn = 6, /**< otbn */
  kTopSaferootPlicPeripheralKeymgr = 7, /**< keymgr */
  kTopSaferootPlicPeripheralAlertHandler = 8, /**< alert_handler */
  kTopSaferootPlicPeripheralOtpCtrl = 9, /**< otp_ctrl */
  kTopSaferootPlicPeripheralSensorCtrl = 10, /**< sensor_ctrl */
  kTopSaferootPlicPeripheralSpiDevice = 11, /**< spi_device */
  kTopSaferootPlicPeripheralGpio = 12, /**< gpio */
  kTopSaferootPlicPeripheralI2c0 = 13, /**< i2c0 */
  kTopSaferootPlicPeripheralUart0 = 14, /**< uart0 */
  kTopSaferootPlicPeripheralRvTimer = 15, /**< rv_timer */
  kTopSaferootPlicPeripheralAonTimerAon = 16, /**< aon_timer_aon */
  kTopSaferootPlicPeripheralPwrmgrAon = 17, /**< pwrmgr_aon */
  kTopSaferootPlicPeripheralFlashCtrl = 18, /**< flash_ctrl */
  kTopSaferootPlicPeripheralLast = 18, /**< \internal Final PLIC peripheral */
} top_saferoot_plic_peripheral_t;

/**
 * PLIC Interrupt Source.
 *
 * Enumeration of all PLIC interrupt sources. The interrupt sources belonging to
 * the same peripheral are guaranteed to be consecutive.
 */
typedef enum top_saferoot_plic_irq_id {
  kTopSaferootPlicIrqIdNone = 0, /**< No Interrupt */
  kTopSaferootPlicIrqIdHmacHmacDone = 1, /**< hmac_hmac_done */
  kTopSaferootPlicIrqIdHmacFifoEmpty = 2, /**< hmac_fifo_empty */
  kTopSaferootPlicIrqIdHmacHmacErr = 3, /**< hmac_hmac_err */
  kTopSaferootPlicIrqIdKmacKmacDone = 4, /**< kmac_kmac_done */
  kTopSaferootPlicIrqIdKmacFifoEmpty = 5, /**< kmac_fifo_empty */
  kTopSaferootPlicIrqIdKmacKmacErr = 6, /**< kmac_kmac_err */
  kTopSaferootPlicIrqIdCsrngCsCmdReqDone = 7, /**< csrng_cs_cmd_req_done */
  kTopSaferootPlicIrqIdCsrngCsEntropyReq = 8, /**< csrng_cs_entropy_req */
  kTopSaferootPlicIrqIdCsrngCsHwInstExc = 9, /**< csrng_cs_hw_inst_exc */
  kTopSaferootPlicIrqIdCsrngCsFatalErr = 10, /**< csrng_cs_fatal_err */
  kTopSaferootPlicIrqIdEntropySrcEsEntropyValid = 11, /**< entropy_src_es_entropy_valid */
  kTopSaferootPlicIrqIdEntropySrcEsHealthTestFailed = 12, /**< entropy_src_es_health_test_failed */
  kTopSaferootPlicIrqIdEntropySrcEsObserveFifoReady = 13, /**< entropy_src_es_observe_fifo_ready */
  kTopSaferootPlicIrqIdEntropySrcEsFatalErr = 14, /**< entropy_src_es_fatal_err */
  kTopSaferootPlicIrqIdEdn0EdnCmdReqDone = 15, /**< edn0_edn_cmd_req_done */
  kTopSaferootPlicIrqIdEdn0EdnFatalErr = 16, /**< edn0_edn_fatal_err */
  kTopSaferootPlicIrqIdOtbnDone = 17, /**< otbn_done */
  kTopSaferootPlicIrqIdKeymgrOpDone = 18, /**< keymgr_op_done */
  kTopSaferootPlicIrqIdAlertHandlerClassa = 19, /**< alert_handler_classa */
  kTopSaferootPlicIrqIdAlertHandlerClassb = 20, /**< alert_handler_classb */
  kTopSaferootPlicIrqIdAlertHandlerClassc = 21, /**< alert_handler_classc */
  kTopSaferootPlicIrqIdAlertHandlerClassd = 22, /**< alert_handler_classd */
  kTopSaferootPlicIrqIdOtpCtrlOtpOperationDone = 23, /**< otp_ctrl_otp_operation_done */
  kTopSaferootPlicIrqIdOtpCtrlOtpError = 24, /**< otp_ctrl_otp_error */
  kTopSaferootPlicIrqIdSensorCtrlIoStatusChange = 25, /**< sensor_ctrl_io_status_change */
  kTopSaferootPlicIrqIdSensorCtrlInitStatusChange = 26, /**< sensor_ctrl_init_status_change */
  kTopSaferootPlicIrqIdSpiDeviceUploadCmdfifoNotEmpty = 27, /**< spi_device_upload_cmdfifo_not_empty */
  kTopSaferootPlicIrqIdSpiDeviceUploadPayloadNotEmpty = 28, /**< spi_device_upload_payload_not_empty */
  kTopSaferootPlicIrqIdSpiDeviceUploadPayloadOverflow = 29, /**< spi_device_upload_payload_overflow */
  kTopSaferootPlicIrqIdSpiDeviceReadbufWatermark = 30, /**< spi_device_readbuf_watermark */
  kTopSaferootPlicIrqIdSpiDeviceReadbufFlip = 31, /**< spi_device_readbuf_flip */
  kTopSaferootPlicIrqIdSpiDeviceTpmHeaderNotEmpty = 32, /**< spi_device_tpm_header_not_empty */
  kTopSaferootPlicIrqIdSpiDeviceTpmRdfifoCmdEnd = 33, /**< spi_device_tpm_rdfifo_cmd_end */
  kTopSaferootPlicIrqIdSpiDeviceTpmRdfifoDrop = 34, /**< spi_device_tpm_rdfifo_drop */
  kTopSaferootPlicIrqIdGpioGpio0 = 35, /**< gpio_gpio 0 */
  kTopSaferootPlicIrqIdGpioGpio1 = 36, /**< gpio_gpio 1 */
  kTopSaferootPlicIrqIdGpioGpio2 = 37, /**< gpio_gpio 2 */
  kTopSaferootPlicIrqIdGpioGpio3 = 38, /**< gpio_gpio 3 */
  kTopSaferootPlicIrqIdGpioGpio4 = 39, /**< gpio_gpio 4 */
  kTopSaferootPlicIrqIdGpioGpio5 = 40, /**< gpio_gpio 5 */
  kTopSaferootPlicIrqIdGpioGpio6 = 41, /**< gpio_gpio 6 */
  kTopSaferootPlicIrqIdGpioGpio7 = 42, /**< gpio_gpio 7 */
  kTopSaferootPlicIrqIdGpioGpio8 = 43, /**< gpio_gpio 8 */
  kTopSaferootPlicIrqIdGpioGpio9 = 44, /**< gpio_gpio 9 */
  kTopSaferootPlicIrqIdGpioGpio10 = 45, /**< gpio_gpio 10 */
  kTopSaferootPlicIrqIdGpioGpio11 = 46, /**< gpio_gpio 11 */
  kTopSaferootPlicIrqIdGpioGpio12 = 47, /**< gpio_gpio 12 */
  kTopSaferootPlicIrqIdGpioGpio13 = 48, /**< gpio_gpio 13 */
  kTopSaferootPlicIrqIdGpioGpio14 = 49, /**< gpio_gpio 14 */
  kTopSaferootPlicIrqIdGpioGpio15 = 50, /**< gpio_gpio 15 */
  kTopSaferootPlicIrqIdGpioGpio16 = 51, /**< gpio_gpio 16 */
  kTopSaferootPlicIrqIdGpioGpio17 = 52, /**< gpio_gpio 17 */
  kTopSaferootPlicIrqIdGpioGpio18 = 53, /**< gpio_gpio 18 */
  kTopSaferootPlicIrqIdGpioGpio19 = 54, /**< gpio_gpio 19 */
  kTopSaferootPlicIrqIdGpioGpio20 = 55, /**< gpio_gpio 20 */
  kTopSaferootPlicIrqIdGpioGpio21 = 56, /**< gpio_gpio 21 */
  kTopSaferootPlicIrqIdGpioGpio22 = 57, /**< gpio_gpio 22 */
  kTopSaferootPlicIrqIdGpioGpio23 = 58, /**< gpio_gpio 23 */
  kTopSaferootPlicIrqIdGpioGpio24 = 59, /**< gpio_gpio 24 */
  kTopSaferootPlicIrqIdGpioGpio25 = 60, /**< gpio_gpio 25 */
  kTopSaferootPlicIrqIdGpioGpio26 = 61, /**< gpio_gpio 26 */
  kTopSaferootPlicIrqIdGpioGpio27 = 62, /**< gpio_gpio 27 */
  kTopSaferootPlicIrqIdGpioGpio28 = 63, /**< gpio_gpio 28 */
  kTopSaferootPlicIrqIdGpioGpio29 = 64, /**< gpio_gpio 29 */
  kTopSaferootPlicIrqIdGpioGpio30 = 65, /**< gpio_gpio 30 */
  kTopSaferootPlicIrqIdGpioGpio31 = 66, /**< gpio_gpio 31 */
  kTopSaferootPlicIrqIdI2c0FmtThreshold = 67, /**< i2c0_fmt_threshold */
  kTopSaferootPlicIrqIdI2c0RxThreshold = 68, /**< i2c0_rx_threshold */
  kTopSaferootPlicIrqIdI2c0AcqThreshold = 69, /**< i2c0_acq_threshold */
  kTopSaferootPlicIrqIdI2c0RxOverflow = 70, /**< i2c0_rx_overflow */
  kTopSaferootPlicIrqIdI2c0ControllerHalt = 71, /**< i2c0_controller_halt */
  kTopSaferootPlicIrqIdI2c0SclInterference = 72, /**< i2c0_scl_interference */
  kTopSaferootPlicIrqIdI2c0SdaInterference = 73, /**< i2c0_sda_interference */
  kTopSaferootPlicIrqIdI2c0StretchTimeout = 74, /**< i2c0_stretch_timeout */
  kTopSaferootPlicIrqIdI2c0SdaUnstable = 75, /**< i2c0_sda_unstable */
  kTopSaferootPlicIrqIdI2c0CmdComplete = 76, /**< i2c0_cmd_complete */
  kTopSaferootPlicIrqIdI2c0TxStretch = 77, /**< i2c0_tx_stretch */
  kTopSaferootPlicIrqIdI2c0TxThreshold = 78, /**< i2c0_tx_threshold */
  kTopSaferootPlicIrqIdI2c0AcqStretch = 79, /**< i2c0_acq_stretch */
  kTopSaferootPlicIrqIdI2c0UnexpStop = 80, /**< i2c0_unexp_stop */
  kTopSaferootPlicIrqIdI2c0HostTimeout = 81, /**< i2c0_host_timeout */
  kTopSaferootPlicIrqIdUart0TxWatermark = 82, /**< uart0_tx_watermark */
  kTopSaferootPlicIrqIdUart0RxWatermark = 83, /**< uart0_rx_watermark */
  kTopSaferootPlicIrqIdUart0TxDone = 84, /**< uart0_tx_done */
  kTopSaferootPlicIrqIdUart0RxOverflow = 85, /**< uart0_rx_overflow */
  kTopSaferootPlicIrqIdUart0RxFrameErr = 86, /**< uart0_rx_frame_err */
  kTopSaferootPlicIrqIdUart0RxBreakErr = 87, /**< uart0_rx_break_err */
  kTopSaferootPlicIrqIdUart0RxTimeout = 88, /**< uart0_rx_timeout */
  kTopSaferootPlicIrqIdUart0RxParityErr = 89, /**< uart0_rx_parity_err */
  kTopSaferootPlicIrqIdUart0TxEmpty = 90, /**< uart0_tx_empty */
  kTopSaferootPlicIrqIdRvTimerTimerExpiredHart0Timer0 = 91, /**< rv_timer_timer_expired_hart0_timer0 */
  kTopSaferootPlicIrqIdAonTimerAonWkupTimerExpired = 92, /**< aon_timer_aon_wkup_timer_expired */
  kTopSaferootPlicIrqIdAonTimerAonWdogTimerBark = 93, /**< aon_timer_aon_wdog_timer_bark */
  kTopSaferootPlicIrqIdPwrmgrAonWakeup = 94, /**< pwrmgr_aon_wakeup */
  kTopSaferootPlicIrqIdFlashCtrlProgEmpty = 95, /**< flash_ctrl_prog_empty */
  kTopSaferootPlicIrqIdFlashCtrlProgLvl = 96, /**< flash_ctrl_prog_lvl */
  kTopSaferootPlicIrqIdFlashCtrlRdFull = 97, /**< flash_ctrl_rd_full */
  kTopSaferootPlicIrqIdFlashCtrlRdLvl = 98, /**< flash_ctrl_rd_lvl */
  kTopSaferootPlicIrqIdFlashCtrlOpDone = 99, /**< flash_ctrl_op_done */
  kTopSaferootPlicIrqIdFlashCtrlCorrErr = 100, /**< flash_ctrl_corr_err */
  kTopSaferootPlicIrqIdLast = 100, /**< \internal The Last Valid Interrupt ID. */
} top_saferoot_plic_irq_id_t;

/**
 * PLIC Interrupt Source to Peripheral Map
 *
 * This array is a mapping from `top_saferoot_plic_irq_id_t` to
 * `top_saferoot_plic_peripheral_t`.
 */
extern const top_saferoot_plic_peripheral_t
    top_saferoot_plic_interrupt_for_peripheral[101];

/**
 * PLIC Interrupt Target.
 *
 * Enumeration used to determine which set of IE, CC, threshold registers to
 * access for a given interrupt target.
 */
typedef enum top_saferoot_plic_target {
  kTopSaferootPlicTargetIbex0 = 0, /**< Ibex Core 0 */
  kTopSaferootPlicTargetLast = 0, /**< \internal Final PLIC target */
} top_saferoot_plic_target_t;


/**
 * Alert Handler Source Peripheral.
 *
 * Enumeration used to determine which peripheral asserted the corresponding
 * alert.
 */
typedef enum top_saferoot_alert_peripheral {
  kTopSaferootAlertPeripheralExternal = 0, /**< External Peripheral */
  kTopSaferootAlertPeripheralAes = 1, /**< aes */
  kTopSaferootAlertPeripheralHmac = 2, /**< hmac */
  kTopSaferootAlertPeripheralKmac = 3, /**< kmac */
  kTopSaferootAlertPeripheralCsrng = 4, /**< csrng */
  kTopSaferootAlertPeripheralEntropySrc = 5, /**< entropy_src */
  kTopSaferootAlertPeripheralEdn0 = 6, /**< edn0 */
  kTopSaferootAlertPeripheralOtbn = 7, /**< otbn */
  kTopSaferootAlertPeripheralKeymgr = 8, /**< keymgr */
  kTopSaferootAlertPeripheralLcCtrl = 9, /**< lc_ctrl */
  kTopSaferootAlertPeripheralOtpCtrl = 10, /**< otp_ctrl */
  kTopSaferootAlertPeripheralSensorCtrl = 11, /**< sensor_ctrl */
  kTopSaferootAlertPeripheralSpiDevice = 12, /**< spi_device */
  kTopSaferootAlertPeripheralGpio = 13, /**< gpio */
  kTopSaferootAlertPeripheralI2c0 = 14, /**< i2c0 */
  kTopSaferootAlertPeripheralUart0 = 15, /**< uart0 */
  kTopSaferootAlertPeripheralRvPlic = 16, /**< rv_plic */
  kTopSaferootAlertPeripheralRvTimer = 17, /**< rv_timer */
  kTopSaferootAlertPeripheralAonTimerAon = 18, /**< aon_timer_aon */
  kTopSaferootAlertPeripheralPwrmgrAon = 19, /**< pwrmgr_aon */
  kTopSaferootAlertPeripheralClkmgrAon = 20, /**< clkmgr_aon */
  kTopSaferootAlertPeripheralRstmgrAon = 21, /**< rstmgr_aon */
  kTopSaferootAlertPeripheralPinmuxAon = 22, /**< pinmux_aon */
  kTopSaferootAlertPeripheralRomCtrl = 23, /**< rom_ctrl */
  kTopSaferootAlertPeripheralSramCtrl = 24, /**< sram_ctrl */
  kTopSaferootAlertPeripheralFlashCtrl = 25, /**< flash_ctrl */
  kTopSaferootAlertPeripheralRvCoreIbex = 26, /**< rv_core_ibex */
  kTopSaferootAlertPeripheralLast = 26, /**< \internal Final Alert peripheral */
} top_saferoot_alert_peripheral_t;

/**
 * Alert Handler Alert Source.
 *
 * Enumeration of all Alert Handler Alert Sources. The alert sources belonging to
 * the same peripheral are guaranteed to be consecutive.
 */
typedef enum top_saferoot_alert_id {
  kTopSaferootAlertIdAesRecovCtrlUpdateErr = 0, /**< aes_recov_ctrl_update_err */
  kTopSaferootAlertIdAesFatalFault = 1, /**< aes_fatal_fault */
  kTopSaferootAlertIdHmacFatalFault = 2, /**< hmac_fatal_fault */
  kTopSaferootAlertIdKmacRecovOperationErr = 3, /**< kmac_recov_operation_err */
  kTopSaferootAlertIdKmacFatalFaultErr = 4, /**< kmac_fatal_fault_err */
  kTopSaferootAlertIdCsrngRecovAlert = 5, /**< csrng_recov_alert */
  kTopSaferootAlertIdCsrngFatalAlert = 6, /**< csrng_fatal_alert */
  kTopSaferootAlertIdEntropySrcRecovAlert = 7, /**< entropy_src_recov_alert */
  kTopSaferootAlertIdEntropySrcFatalAlert = 8, /**< entropy_src_fatal_alert */
  kTopSaferootAlertIdEdn0RecovAlert = 9, /**< edn0_recov_alert */
  kTopSaferootAlertIdEdn0FatalAlert = 10, /**< edn0_fatal_alert */
  kTopSaferootAlertIdOtbnFatal = 11, /**< otbn_fatal */
  kTopSaferootAlertIdOtbnRecov = 12, /**< otbn_recov */
  kTopSaferootAlertIdKeymgrRecovOperationErr = 13, /**< keymgr_recov_operation_err */
  kTopSaferootAlertIdKeymgrFatalFaultErr = 14, /**< keymgr_fatal_fault_err */
  kTopSaferootAlertIdLcCtrlFatalProgError = 15, /**< lc_ctrl_fatal_prog_error */
  kTopSaferootAlertIdLcCtrlFatalStateError = 16, /**< lc_ctrl_fatal_state_error */
  kTopSaferootAlertIdLcCtrlFatalBusIntegError = 17, /**< lc_ctrl_fatal_bus_integ_error */
  kTopSaferootAlertIdOtpCtrlFatalMacroError = 18, /**< otp_ctrl_fatal_macro_error */
  kTopSaferootAlertIdOtpCtrlFatalCheckError = 19, /**< otp_ctrl_fatal_check_error */
  kTopSaferootAlertIdOtpCtrlFatalBusIntegError = 20, /**< otp_ctrl_fatal_bus_integ_error */
  kTopSaferootAlertIdOtpCtrlFatalPrimOtpAlert = 21, /**< otp_ctrl_fatal_prim_otp_alert */
  kTopSaferootAlertIdOtpCtrlRecovPrimOtpAlert = 22, /**< otp_ctrl_recov_prim_otp_alert */
  kTopSaferootAlertIdSensorCtrlRecovAlert = 23, /**< sensor_ctrl_recov_alert */
  kTopSaferootAlertIdSensorCtrlFatalAlert = 24, /**< sensor_ctrl_fatal_alert */
  kTopSaferootAlertIdSpiDeviceFatalFault = 25, /**< spi_device_fatal_fault */
  kTopSaferootAlertIdGpioFatalFault = 26, /**< gpio_fatal_fault */
  kTopSaferootAlertIdI2c0FatalFault = 27, /**< i2c0_fatal_fault */
  kTopSaferootAlertIdUart0FatalFault = 28, /**< uart0_fatal_fault */
  kTopSaferootAlertIdRvPlicFatalFault = 29, /**< rv_plic_fatal_fault */
  kTopSaferootAlertIdRvTimerFatalFault = 30, /**< rv_timer_fatal_fault */
  kTopSaferootAlertIdAonTimerAonFatalFault = 31, /**< aon_timer_aon_fatal_fault */
  kTopSaferootAlertIdPwrmgrAonFatalFault = 32, /**< pwrmgr_aon_fatal_fault */
  kTopSaferootAlertIdClkmgrAonRecovFault = 33, /**< clkmgr_aon_recov_fault */
  kTopSaferootAlertIdClkmgrAonFatalFault = 34, /**< clkmgr_aon_fatal_fault */
  kTopSaferootAlertIdRstmgrAonFatalFault = 35, /**< rstmgr_aon_fatal_fault */
  kTopSaferootAlertIdRstmgrAonFatalCnstyFault = 36, /**< rstmgr_aon_fatal_cnsty_fault */
  kTopSaferootAlertIdPinmuxAonFatalFault = 37, /**< pinmux_aon_fatal_fault */
  kTopSaferootAlertIdRomCtrlFatal = 38, /**< rom_ctrl_fatal */
  kTopSaferootAlertIdSramCtrlFatalError = 39, /**< sram_ctrl_fatal_error */
  kTopSaferootAlertIdFlashCtrlRecovErr = 40, /**< flash_ctrl_recov_err */
  kTopSaferootAlertIdFlashCtrlFatalStdErr = 41, /**< flash_ctrl_fatal_std_err */
  kTopSaferootAlertIdFlashCtrlFatalErr = 42, /**< flash_ctrl_fatal_err */
  kTopSaferootAlertIdFlashCtrlFatalPrimFlashAlert = 43, /**< flash_ctrl_fatal_prim_flash_alert */
  kTopSaferootAlertIdFlashCtrlRecovPrimFlashAlert = 44, /**< flash_ctrl_recov_prim_flash_alert */
  kTopSaferootAlertIdRvCoreIbexFatalSwErr = 45, /**< rv_core_ibex_fatal_sw_err */
  kTopSaferootAlertIdRvCoreIbexRecovSwErr = 46, /**< rv_core_ibex_recov_sw_err */
  kTopSaferootAlertIdRvCoreIbexFatalHwErr = 47, /**< rv_core_ibex_fatal_hw_err */
  kTopSaferootAlertIdRvCoreIbexRecovHwErr = 48, /**< rv_core_ibex_recov_hw_err */
  kTopSaferootAlertIdLast = 48, /**< \internal The Last Valid Alert ID. */
} top_saferoot_alert_id_t;

/**
 * Alert Handler Alert Source to Peripheral Map
 *
 * This array is a mapping from `top_saferoot_alert_id_t` to
 * `top_saferoot_alert_peripheral_t`.
 */
extern const top_saferoot_alert_peripheral_t
    top_saferoot_alert_for_peripheral[49];

#define PINMUX_MIO_PERIPH_INSEL_IDX_OFFSET 2

// PERIPH_INSEL ranges from 0 to NUM_MIO_PADS + 2 -1}
//  0 and 1 are tied to value 0 and 1
#define NUM_MIO_PADS 16
#define NUM_DIO_PADS 6

#define PINMUX_PERIPH_OUTSEL_IDX_OFFSET 3

/**
 * Pinmux Peripheral Input.
 */
typedef enum top_saferoot_pinmux_peripheral_in {
  kTopSaferootPinmuxPeripheralInGpioGpio0 = 0, /**< Peripheral Input 0 */
  kTopSaferootPinmuxPeripheralInGpioGpio1 = 1, /**< Peripheral Input 1 */
  kTopSaferootPinmuxPeripheralInGpioGpio2 = 2, /**< Peripheral Input 2 */
  kTopSaferootPinmuxPeripheralInGpioGpio3 = 3, /**< Peripheral Input 3 */
  kTopSaferootPinmuxPeripheralInGpioGpio4 = 4, /**< Peripheral Input 4 */
  kTopSaferootPinmuxPeripheralInGpioGpio5 = 5, /**< Peripheral Input 5 */
  kTopSaferootPinmuxPeripheralInGpioGpio6 = 6, /**< Peripheral Input 6 */
  kTopSaferootPinmuxPeripheralInGpioGpio7 = 7, /**< Peripheral Input 7 */
  kTopSaferootPinmuxPeripheralInGpioGpio8 = 8, /**< Peripheral Input 8 */
  kTopSaferootPinmuxPeripheralInGpioGpio9 = 9, /**< Peripheral Input 9 */
  kTopSaferootPinmuxPeripheralInGpioGpio10 = 10, /**< Peripheral Input 10 */
  kTopSaferootPinmuxPeripheralInGpioGpio11 = 11, /**< Peripheral Input 11 */
  kTopSaferootPinmuxPeripheralInGpioGpio12 = 12, /**< Peripheral Input 12 */
  kTopSaferootPinmuxPeripheralInGpioGpio13 = 13, /**< Peripheral Input 13 */
  kTopSaferootPinmuxPeripheralInGpioGpio14 = 14, /**< Peripheral Input 14 */
  kTopSaferootPinmuxPeripheralInGpioGpio15 = 15, /**< Peripheral Input 15 */
  kTopSaferootPinmuxPeripheralInGpioGpio16 = 16, /**< Peripheral Input 16 */
  kTopSaferootPinmuxPeripheralInGpioGpio17 = 17, /**< Peripheral Input 17 */
  kTopSaferootPinmuxPeripheralInGpioGpio18 = 18, /**< Peripheral Input 18 */
  kTopSaferootPinmuxPeripheralInGpioGpio19 = 19, /**< Peripheral Input 19 */
  kTopSaferootPinmuxPeripheralInGpioGpio20 = 20, /**< Peripheral Input 20 */
  kTopSaferootPinmuxPeripheralInGpioGpio21 = 21, /**< Peripheral Input 21 */
  kTopSaferootPinmuxPeripheralInGpioGpio22 = 22, /**< Peripheral Input 22 */
  kTopSaferootPinmuxPeripheralInGpioGpio23 = 23, /**< Peripheral Input 23 */
  kTopSaferootPinmuxPeripheralInGpioGpio24 = 24, /**< Peripheral Input 24 */
  kTopSaferootPinmuxPeripheralInGpioGpio25 = 25, /**< Peripheral Input 25 */
  kTopSaferootPinmuxPeripheralInGpioGpio26 = 26, /**< Peripheral Input 26 */
  kTopSaferootPinmuxPeripheralInGpioGpio27 = 27, /**< Peripheral Input 27 */
  kTopSaferootPinmuxPeripheralInGpioGpio28 = 28, /**< Peripheral Input 28 */
  kTopSaferootPinmuxPeripheralInGpioGpio29 = 29, /**< Peripheral Input 29 */
  kTopSaferootPinmuxPeripheralInGpioGpio30 = 30, /**< Peripheral Input 30 */
  kTopSaferootPinmuxPeripheralInGpioGpio31 = 31, /**< Peripheral Input 31 */
  kTopSaferootPinmuxPeripheralInI2c0Sda = 32, /**< Peripheral Input 32 */
  kTopSaferootPinmuxPeripheralInI2c0Scl = 33, /**< Peripheral Input 33 */
  kTopSaferootPinmuxPeripheralInUart0Rx = 34, /**< Peripheral Input 34 */
  kTopSaferootPinmuxPeripheralInSpiDeviceTpmCsb = 35, /**< Peripheral Input 35 */
  kTopSaferootPinmuxPeripheralInFlashCtrlTck = 36, /**< Peripheral Input 36 */
  kTopSaferootPinmuxPeripheralInFlashCtrlTms = 37, /**< Peripheral Input 37 */
  kTopSaferootPinmuxPeripheralInFlashCtrlTdi = 38, /**< Peripheral Input 38 */
  kTopSaferootPinmuxPeripheralInLast = 38, /**< \internal Last valid peripheral input */
} top_saferoot_pinmux_peripheral_in_t;

/**
 * Pinmux MIO Input Selector.
 */
typedef enum top_saferoot_pinmux_insel {
  kTopSaferootPinmuxInselConstantZero = 0, /**< Tie constantly to zero */
  kTopSaferootPinmuxInselConstantOne = 1, /**< Tie constantly to one */
  kTopSaferootPinmuxInselMio0 = 2, /**< MIO Pad 0 */
  kTopSaferootPinmuxInselMio1 = 3, /**< MIO Pad 1 */
  kTopSaferootPinmuxInselMio2 = 4, /**< MIO Pad 2 */
  kTopSaferootPinmuxInselMio3 = 5, /**< MIO Pad 3 */
  kTopSaferootPinmuxInselMio4 = 6, /**< MIO Pad 4 */
  kTopSaferootPinmuxInselMio5 = 7, /**< MIO Pad 5 */
  kTopSaferootPinmuxInselMio6 = 8, /**< MIO Pad 6 */
  kTopSaferootPinmuxInselMio7 = 9, /**< MIO Pad 7 */
  kTopSaferootPinmuxInselMio8 = 10, /**< MIO Pad 8 */
  kTopSaferootPinmuxInselMio9 = 11, /**< MIO Pad 9 */
  kTopSaferootPinmuxInselMio10 = 12, /**< MIO Pad 10 */
  kTopSaferootPinmuxInselMio11 = 13, /**< MIO Pad 11 */
  kTopSaferootPinmuxInselMio12 = 14, /**< MIO Pad 12 */
  kTopSaferootPinmuxInselMio13 = 15, /**< MIO Pad 13 */
  kTopSaferootPinmuxInselMio14 = 16, /**< MIO Pad 14 */
  kTopSaferootPinmuxInselMio15 = 17, /**< MIO Pad 15 */
  kTopSaferootPinmuxInselLast = 17, /**< \internal Last valid insel value */
} top_saferoot_pinmux_insel_t;

/**
 * Pinmux MIO Output.
 */
typedef enum top_saferoot_pinmux_mio_out {
  kTopSaferootPinmuxMioOutMio0 = 0, /**< MIO Pad 0 */
  kTopSaferootPinmuxMioOutMio1 = 1, /**< MIO Pad 1 */
  kTopSaferootPinmuxMioOutMio2 = 2, /**< MIO Pad 2 */
  kTopSaferootPinmuxMioOutMio3 = 3, /**< MIO Pad 3 */
  kTopSaferootPinmuxMioOutMio4 = 4, /**< MIO Pad 4 */
  kTopSaferootPinmuxMioOutMio5 = 5, /**< MIO Pad 5 */
  kTopSaferootPinmuxMioOutMio6 = 6, /**< MIO Pad 6 */
  kTopSaferootPinmuxMioOutMio7 = 7, /**< MIO Pad 7 */
  kTopSaferootPinmuxMioOutMio8 = 8, /**< MIO Pad 8 */
  kTopSaferootPinmuxMioOutMio9 = 9, /**< MIO Pad 9 */
  kTopSaferootPinmuxMioOutMio10 = 10, /**< MIO Pad 10 */
  kTopSaferootPinmuxMioOutMio11 = 11, /**< MIO Pad 11 */
  kTopSaferootPinmuxMioOutMio12 = 12, /**< MIO Pad 12 */
  kTopSaferootPinmuxMioOutMio13 = 13, /**< MIO Pad 13 */
  kTopSaferootPinmuxMioOutMio14 = 14, /**< MIO Pad 14 */
  kTopSaferootPinmuxMioOutMio15 = 15, /**< MIO Pad 15 */
  kTopSaferootPinmuxMioOutLast = 15, /**< \internal Last valid mio output */
} top_saferoot_pinmux_mio_out_t;

/**
 * Pinmux Peripheral Output Selector.
 */
typedef enum top_saferoot_pinmux_outsel {
  kTopSaferootPinmuxOutselConstantZero = 0, /**< Tie constantly to zero */
  kTopSaferootPinmuxOutselConstantOne = 1, /**< Tie constantly to one */
  kTopSaferootPinmuxOutselConstantHighZ = 2, /**< Tie constantly to high-Z */
  kTopSaferootPinmuxOutselGpioGpio0 = 3, /**< Peripheral Output 0 */
  kTopSaferootPinmuxOutselGpioGpio1 = 4, /**< Peripheral Output 1 */
  kTopSaferootPinmuxOutselGpioGpio2 = 5, /**< Peripheral Output 2 */
  kTopSaferootPinmuxOutselGpioGpio3 = 6, /**< Peripheral Output 3 */
  kTopSaferootPinmuxOutselGpioGpio4 = 7, /**< Peripheral Output 4 */
  kTopSaferootPinmuxOutselGpioGpio5 = 8, /**< Peripheral Output 5 */
  kTopSaferootPinmuxOutselGpioGpio6 = 9, /**< Peripheral Output 6 */
  kTopSaferootPinmuxOutselGpioGpio7 = 10, /**< Peripheral Output 7 */
  kTopSaferootPinmuxOutselGpioGpio8 = 11, /**< Peripheral Output 8 */
  kTopSaferootPinmuxOutselGpioGpio9 = 12, /**< Peripheral Output 9 */
  kTopSaferootPinmuxOutselGpioGpio10 = 13, /**< Peripheral Output 10 */
  kTopSaferootPinmuxOutselGpioGpio11 = 14, /**< Peripheral Output 11 */
  kTopSaferootPinmuxOutselGpioGpio12 = 15, /**< Peripheral Output 12 */
  kTopSaferootPinmuxOutselGpioGpio13 = 16, /**< Peripheral Output 13 */
  kTopSaferootPinmuxOutselGpioGpio14 = 17, /**< Peripheral Output 14 */
  kTopSaferootPinmuxOutselGpioGpio15 = 18, /**< Peripheral Output 15 */
  kTopSaferootPinmuxOutselGpioGpio16 = 19, /**< Peripheral Output 16 */
  kTopSaferootPinmuxOutselGpioGpio17 = 20, /**< Peripheral Output 17 */
  kTopSaferootPinmuxOutselGpioGpio18 = 21, /**< Peripheral Output 18 */
  kTopSaferootPinmuxOutselGpioGpio19 = 22, /**< Peripheral Output 19 */
  kTopSaferootPinmuxOutselGpioGpio20 = 23, /**< Peripheral Output 20 */
  kTopSaferootPinmuxOutselGpioGpio21 = 24, /**< Peripheral Output 21 */
  kTopSaferootPinmuxOutselGpioGpio22 = 25, /**< Peripheral Output 22 */
  kTopSaferootPinmuxOutselGpioGpio23 = 26, /**< Peripheral Output 23 */
  kTopSaferootPinmuxOutselGpioGpio24 = 27, /**< Peripheral Output 24 */
  kTopSaferootPinmuxOutselGpioGpio25 = 28, /**< Peripheral Output 25 */
  kTopSaferootPinmuxOutselGpioGpio26 = 29, /**< Peripheral Output 26 */
  kTopSaferootPinmuxOutselGpioGpio27 = 30, /**< Peripheral Output 27 */
  kTopSaferootPinmuxOutselGpioGpio28 = 31, /**< Peripheral Output 28 */
  kTopSaferootPinmuxOutselGpioGpio29 = 32, /**< Peripheral Output 29 */
  kTopSaferootPinmuxOutselGpioGpio30 = 33, /**< Peripheral Output 30 */
  kTopSaferootPinmuxOutselGpioGpio31 = 34, /**< Peripheral Output 31 */
  kTopSaferootPinmuxOutselI2c0Sda = 35, /**< Peripheral Output 32 */
  kTopSaferootPinmuxOutselI2c0Scl = 36, /**< Peripheral Output 33 */
  kTopSaferootPinmuxOutselUart0Tx = 37, /**< Peripheral Output 34 */
  kTopSaferootPinmuxOutselFlashCtrlTdo = 38, /**< Peripheral Output 35 */
  kTopSaferootPinmuxOutselSensorCtrlAstDebugOut0 = 39, /**< Peripheral Output 36 */
  kTopSaferootPinmuxOutselSensorCtrlAstDebugOut1 = 40, /**< Peripheral Output 37 */
  kTopSaferootPinmuxOutselSensorCtrlAstDebugOut2 = 41, /**< Peripheral Output 38 */
  kTopSaferootPinmuxOutselSensorCtrlAstDebugOut3 = 42, /**< Peripheral Output 39 */
  kTopSaferootPinmuxOutselSensorCtrlAstDebugOut4 = 43, /**< Peripheral Output 40 */
  kTopSaferootPinmuxOutselSensorCtrlAstDebugOut5 = 44, /**< Peripheral Output 41 */
  kTopSaferootPinmuxOutselSensorCtrlAstDebugOut6 = 45, /**< Peripheral Output 42 */
  kTopSaferootPinmuxOutselSensorCtrlAstDebugOut7 = 46, /**< Peripheral Output 43 */
  kTopSaferootPinmuxOutselSensorCtrlAstDebugOut8 = 47, /**< Peripheral Output 44 */
  kTopSaferootPinmuxOutselOtpMacroTest0 = 48, /**< Peripheral Output 45 */
  kTopSaferootPinmuxOutselLast = 48, /**< \internal Last valid outsel value */
} top_saferoot_pinmux_outsel_t;

/**
 * Dedicated Pad Selects
 */
typedef enum top_saferoot_direct_pads {
  kTopSaferootDirectPadsSpiDeviceSd0 = 0, /**<  */
  kTopSaferootDirectPadsSpiDeviceSd1 = 1, /**<  */
  kTopSaferootDirectPadsSpiDeviceSd2 = 2, /**<  */
  kTopSaferootDirectPadsSpiDeviceSd3 = 3, /**<  */
  kTopSaferootDirectPadsSpiDeviceSck = 4, /**<  */
  kTopSaferootDirectPadsSpiDeviceCsb = 5, /**<  */
  kTopSaferootDirectPadsLast = 5, /**< \internal Last valid direct pad */
} top_saferoot_direct_pads_t;

/**
 * Muxed Pad Selects
 */
typedef enum top_saferoot_muxed_pads {
  kTopSaferootMuxedPadsMio0 = 0, /**<  */
  kTopSaferootMuxedPadsMio1 = 1, /**<  */
  kTopSaferootMuxedPadsMio2 = 2, /**<  */
  kTopSaferootMuxedPadsMio3 = 3, /**<  */
  kTopSaferootMuxedPadsMio4 = 4, /**<  */
  kTopSaferootMuxedPadsMio5 = 5, /**<  */
  kTopSaferootMuxedPadsMio6 = 6, /**<  */
  kTopSaferootMuxedPadsMio7 = 7, /**<  */
  kTopSaferootMuxedPadsMio8 = 8, /**<  */
  kTopSaferootMuxedPadsMio9 = 9, /**<  */
  kTopSaferootMuxedPadsMio10 = 10, /**<  */
  kTopSaferootMuxedPadsMio11 = 11, /**<  */
  kTopSaferootMuxedPadsMio12 = 12, /**<  */
  kTopSaferootMuxedPadsMio13 = 13, /**<  */
  kTopSaferootMuxedPadsMio14 = 14, /**<  */
  kTopSaferootMuxedPadsMio15 = 15, /**<  */
  kTopSaferootMuxedPadsLast = 15, /**< \internal Last valid muxed pad */
} top_saferoot_muxed_pads_t;

/**
 * Power Manager Wakeup Signals
 */
typedef enum top_saferoot_power_manager_wake_ups {
  kTopSaferootPowerManagerWakeUpsSensorCtrlWkupReq = 0, /**<  */
  kTopSaferootPowerManagerWakeUpsAonTimerAonWkupReq = 1, /**<  */
  kTopSaferootPowerManagerWakeUpsPinmuxAonPinWkupReq = 2, /**<  */
  kTopSaferootPowerManagerWakeUpsLast = 2, /**< \internal Last valid pwrmgr wakeup signal */
} top_saferoot_power_manager_wake_ups_t;

/**
 * Reset Manager Software Controlled Resets
 */
typedef enum top_saferoot_reset_manager_sw_resets {
  kTopSaferootResetManagerSwResetsSpiDevice = 0, /**<  */
  kTopSaferootResetManagerSwResetsI2c0 = 1, /**<  */
  kTopSaferootResetManagerSwResetsLast = 1, /**< \internal Last valid rstmgr software reset request */
} top_saferoot_reset_manager_sw_resets_t;

/**
 * Power Manager Reset Request Signals
 */
typedef enum top_saferoot_power_manager_reset_requests {
  kTopSaferootPowerManagerResetRequestsAonTimerAonAonTimerRstReq = 0, /**<  */
  kTopSaferootPowerManagerResetRequestsLast = 0, /**< \internal Last valid pwrmgr reset_request signal */
} top_saferoot_power_manager_reset_requests_t;

/**
 * Clock Manager Software-Controlled ("Gated") Clocks.
 *
 * The Software has full control over these clocks.
 */
typedef enum top_saferoot_gateable_clocks {
  kTopSaferootGateableClocksIoDiv4Peri = 0, /**< Clock clk_io_div4_peri in group peri */
  kTopSaferootGateableClocksIoDiv2Peri = 1, /**< Clock clk_io_div2_peri in group peri */
  kTopSaferootGateableClocksLast = 1, /**< \internal Last Valid Gateable Clock */
} top_saferoot_gateable_clocks_t;

/**
 * Clock Manager Software-Hinted Clocks.
 *
 * The Software has partial control over these clocks. It can ask them to stop,
 * but the clock manager is in control of whether the clock actually is stopped.
 */
typedef enum top_saferoot_hintable_clocks {
  kTopSaferootHintableClocksMainAes = 0, /**< Clock clk_main_aes in group trans */
  kTopSaferootHintableClocksMainHmac = 1, /**< Clock clk_main_hmac in group trans */
  kTopSaferootHintableClocksMainKmac = 2, /**< Clock clk_main_kmac in group trans */
  kTopSaferootHintableClocksMainOtbn = 3, /**< Clock clk_main_otbn in group trans */
  kTopSaferootHintableClocksLast = 3, /**< \internal Last Valid Hintable Clock */
} top_saferoot_hintable_clocks_t;

/**
 * MMIO Region
 *
 * MMIO region for SafeRoot. Excludes ROM, SRAM, and flash data memory.
 */
#define TOP_SAFEROOT_MMIO_BASE_ADDR 0x40000000u
#define TOP_SAFEROOT_MMIO_SIZE_BYTES 0x10000000u

// Header Extern Guard
#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // OPENTITAN_HW_TOP_SAFEROOT_SW_AUTOGEN_TOP_SAFEROOT_H_
