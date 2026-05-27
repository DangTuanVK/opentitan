// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// ------------------- W A R N I N G: A U T O - G E N E R A T E D   C O D E !! -------------------//
// PLEASE DO NOT HAND-EDIT THIS FILE. IT HAS BEEN AUTO-GENERATED WITH THE FOLLOWING COMMAND:
// util/topgen.py -t hw/top_saferoot/data/top_saferoot.hjson
//                -o hw/top_saferoot/

#ifndef OPENTITAN_HW_TOP_SAFEROOT_SW_AUTOGEN_TOP_SAFEROOT_MEMORY_H_
#define OPENTITAN_HW_TOP_SAFEROOT_SW_AUTOGEN_TOP_SAFEROOT_MEMORY_H_

/**
 * @file
 * @brief Assembler-only Top-Specific Definitions.
 *
 * This file contains preprocessor definitions for use within assembly code.
 *
 * These are not shared with C/C++ code because these are only allowed to be
 * preprocessor definitions, no data or type declarations are allowed. The
 * assembler is also stricter about literals (not allowing suffixes for
 * signed/unsigned which are sensible to use for unsigned values in C/C++).
 */

// Include guard for assembler
#ifdef __ASSEMBLER__

/**
 * Memory base for rom memory on rom_ctrl in top saferoot.
 */
#define TOP_SAFEROOT_ROM_CTRL_ROM_BASE_ADDR 0x0

/**
 * Memory size for rom memory on rom_ctrl in top saferoot.
 */
#define TOP_SAFEROOT_ROM_CTRL_ROM_SIZE_BYTES 0x8000

/**
 * Memory base for ram memory on sram_ctrl in top saferoot.
 */
#define TOP_SAFEROOT_SRAM_CTRL_RAM_BASE_ADDR 0x10000000

/**
 * Memory size for ram memory on sram_ctrl in top saferoot.
 */
#define TOP_SAFEROOT_SRAM_CTRL_RAM_SIZE_BYTES 0x20000

/**
 * Memory base for mem memory on flash_ctrl in top saferoot.
 */
#define TOP_SAFEROOT_FLASH_CTRL_MEM_BASE_ADDR 0x20000000

/**
 * Memory size for mem memory on flash_ctrl in top saferoot.
 */
#define TOP_SAFEROOT_FLASH_CTRL_MEM_SIZE_BYTES 0x100000


/**
 * Peripheral base address for aes in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_AES_BASE_ADDR 0x40000000

/**
 * Peripheral size for aes in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_AES_BASE_ADDR and
 * `TOP_SAFEROOT_AES_BASE_ADDR + TOP_SAFEROOT_AES_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_AES_SIZE_BYTES 0x100
/**
 * Peripheral base address for hmac in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_HMAC_BASE_ADDR 0x40010000

/**
 * Peripheral size for hmac in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_HMAC_BASE_ADDR and
 * `TOP_SAFEROOT_HMAC_BASE_ADDR + TOP_SAFEROOT_HMAC_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_HMAC_SIZE_BYTES 0x2000
/**
 * Peripheral base address for kmac in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_KMAC_BASE_ADDR 0x40020000

/**
 * Peripheral size for kmac in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_KMAC_BASE_ADDR and
 * `TOP_SAFEROOT_KMAC_BASE_ADDR + TOP_SAFEROOT_KMAC_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_KMAC_SIZE_BYTES 0x1000
/**
 * Peripheral base address for csrng in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_CSRNG_BASE_ADDR 0x40030000

/**
 * Peripheral size for csrng in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_CSRNG_BASE_ADDR and
 * `TOP_SAFEROOT_CSRNG_BASE_ADDR + TOP_SAFEROOT_CSRNG_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_CSRNG_SIZE_BYTES 0x80
/**
 * Peripheral base address for entropy_src in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_ENTROPY_SRC_BASE_ADDR 0x40040000

/**
 * Peripheral size for entropy_src in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_ENTROPY_SRC_BASE_ADDR and
 * `TOP_SAFEROOT_ENTROPY_SRC_BASE_ADDR + TOP_SAFEROOT_ENTROPY_SRC_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_ENTROPY_SRC_SIZE_BYTES 0x100
/**
 * Peripheral base address for edn0 in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_EDN0_BASE_ADDR 0x40005000

/**
 * Peripheral size for edn0 in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_EDN0_BASE_ADDR and
 * `TOP_SAFEROOT_EDN0_BASE_ADDR + TOP_SAFEROOT_EDN0_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_EDN0_SIZE_BYTES 0x80
/**
 * Peripheral base address for otbn in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_OTBN_BASE_ADDR 0x40080000

/**
 * Peripheral size for otbn in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_OTBN_BASE_ADDR and
 * `TOP_SAFEROOT_OTBN_BASE_ADDR + TOP_SAFEROOT_OTBN_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_OTBN_SIZE_BYTES 0x10000
/**
 * Peripheral base address for keymgr in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_KEYMGR_BASE_ADDR 0x40100000

/**
 * Peripheral size for keymgr in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_KEYMGR_BASE_ADDR and
 * `TOP_SAFEROOT_KEYMGR_BASE_ADDR + TOP_SAFEROOT_KEYMGR_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_KEYMGR_SIZE_BYTES 0x100
/**
 * Peripheral base address for regs device on lc_ctrl in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_LC_CTRL_REGS_BASE_ADDR 0x40101000

/**
 * Peripheral size for regs device on lc_ctrl in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_LC_CTRL_REGS_BASE_ADDR and
 * `TOP_SAFEROOT_LC_CTRL_REGS_BASE_ADDR + TOP_SAFEROOT_LC_CTRL_REGS_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_LC_CTRL_REGS_SIZE_BYTES 0x100
/**
 * Peripheral base address for dmi device on lc_ctrl in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_LC_CTRL_DMI_BASE_ADDR 0x0

/**
 * Peripheral size for dmi device on lc_ctrl in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_LC_CTRL_DMI_BASE_ADDR and
 * `TOP_SAFEROOT_LC_CTRL_DMI_BASE_ADDR + TOP_SAFEROOT_LC_CTRL_DMI_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_LC_CTRL_DMI_SIZE_BYTES 0x1000
/**
 * Peripheral base address for alert_handler in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_ALERT_HANDLER_BASE_ADDR 0x40120000

/**
 * Peripheral size for alert_handler in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_ALERT_HANDLER_BASE_ADDR and
 * `TOP_SAFEROOT_ALERT_HANDLER_BASE_ADDR + TOP_SAFEROOT_ALERT_HANDLER_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_ALERT_HANDLER_SIZE_BYTES 0x800
/**
 * Peripheral base address for core device on otp_ctrl in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_OTP_CTRL_CORE_BASE_ADDR 0x40130000

/**
 * Peripheral size for core device on otp_ctrl in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_OTP_CTRL_CORE_BASE_ADDR and
 * `TOP_SAFEROOT_OTP_CTRL_CORE_BASE_ADDR + TOP_SAFEROOT_OTP_CTRL_CORE_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_OTP_CTRL_CORE_SIZE_BYTES 0x1000
/**
 * Peripheral base address for prim device on otp_macro in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_OTP_MACRO_PRIM_BASE_ADDR 0x40138000

/**
 * Peripheral size for prim device on otp_macro in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_OTP_MACRO_PRIM_BASE_ADDR and
 * `TOP_SAFEROOT_OTP_MACRO_PRIM_BASE_ADDR + TOP_SAFEROOT_OTP_MACRO_PRIM_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_OTP_MACRO_PRIM_SIZE_BYTES 0x20
/**
 * Peripheral base address for sensor_ctrl in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_SENSOR_CTRL_BASE_ADDR 0x40140000

/**
 * Peripheral size for sensor_ctrl in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_SENSOR_CTRL_BASE_ADDR and
 * `TOP_SAFEROOT_SENSOR_CTRL_BASE_ADDR + TOP_SAFEROOT_SENSOR_CTRL_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_SENSOR_CTRL_SIZE_BYTES 0x80
/**
 * Peripheral base address for spi_device in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_SPI_DEVICE_BASE_ADDR 0x40200000

/**
 * Peripheral size for spi_device in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_SPI_DEVICE_BASE_ADDR and
 * `TOP_SAFEROOT_SPI_DEVICE_BASE_ADDR + TOP_SAFEROOT_SPI_DEVICE_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_SPI_DEVICE_SIZE_BYTES 0x2000
/**
 * Peripheral base address for gpio in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_GPIO_BASE_ADDR 0x40210000

/**
 * Peripheral size for gpio in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_GPIO_BASE_ADDR and
 * `TOP_SAFEROOT_GPIO_BASE_ADDR + TOP_SAFEROOT_GPIO_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_GPIO_SIZE_BYTES 0x80
/**
 * Peripheral base address for i2c0 in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_I2C0_BASE_ADDR 0x40220000

/**
 * Peripheral size for i2c0 in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_I2C0_BASE_ADDR and
 * `TOP_SAFEROOT_I2C0_BASE_ADDR + TOP_SAFEROOT_I2C0_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_I2C0_SIZE_BYTES 0x80
/**
 * Peripheral base address for uart0 in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_UART0_BASE_ADDR 0x40230000

/**
 * Peripheral size for uart0 in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_UART0_BASE_ADDR and
 * `TOP_SAFEROOT_UART0_BASE_ADDR + TOP_SAFEROOT_UART0_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_UART0_SIZE_BYTES 0x40
/**
 * Peripheral base address for rv_plic in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_RV_PLIC_BASE_ADDR 0x48000000

/**
 * Peripheral size for rv_plic in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_RV_PLIC_BASE_ADDR and
 * `TOP_SAFEROOT_RV_PLIC_BASE_ADDR + TOP_SAFEROOT_RV_PLIC_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_RV_PLIC_SIZE_BYTES 0x8000000
/**
 * Peripheral base address for rv_timer in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_RV_TIMER_BASE_ADDR 0x40310000

/**
 * Peripheral size for rv_timer in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_RV_TIMER_BASE_ADDR and
 * `TOP_SAFEROOT_RV_TIMER_BASE_ADDR + TOP_SAFEROOT_RV_TIMER_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_RV_TIMER_SIZE_BYTES 0x200
/**
 * Peripheral base address for aon_timer_aon in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_AON_TIMER_AON_BASE_ADDR 0x40320000

/**
 * Peripheral size for aon_timer_aon in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_AON_TIMER_AON_BASE_ADDR and
 * `TOP_SAFEROOT_AON_TIMER_AON_BASE_ADDR + TOP_SAFEROOT_AON_TIMER_AON_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_AON_TIMER_AON_SIZE_BYTES 0x40
/**
 * Peripheral base address for pwrmgr_aon in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_PWRMGR_AON_BASE_ADDR 0x40330000

/**
 * Peripheral size for pwrmgr_aon in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_PWRMGR_AON_BASE_ADDR and
 * `TOP_SAFEROOT_PWRMGR_AON_BASE_ADDR + TOP_SAFEROOT_PWRMGR_AON_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_PWRMGR_AON_SIZE_BYTES 0x80
/**
 * Peripheral base address for clkmgr_aon in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_CLKMGR_AON_BASE_ADDR 0x40340000

/**
 * Peripheral size for clkmgr_aon in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_CLKMGR_AON_BASE_ADDR and
 * `TOP_SAFEROOT_CLKMGR_AON_BASE_ADDR + TOP_SAFEROOT_CLKMGR_AON_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_CLKMGR_AON_SIZE_BYTES 0x40
/**
 * Peripheral base address for rstmgr_aon in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_RSTMGR_AON_BASE_ADDR 0x40350000

/**
 * Peripheral size for rstmgr_aon in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_RSTMGR_AON_BASE_ADDR and
 * `TOP_SAFEROOT_RSTMGR_AON_BASE_ADDR + TOP_SAFEROOT_RSTMGR_AON_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_RSTMGR_AON_SIZE_BYTES 0x40
/**
 * Peripheral base address for pinmux_aon in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_PINMUX_AON_BASE_ADDR 0x40360000

/**
 * Peripheral size for pinmux_aon in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_PINMUX_AON_BASE_ADDR and
 * `TOP_SAFEROOT_PINMUX_AON_BASE_ADDR + TOP_SAFEROOT_PINMUX_AON_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_PINMUX_AON_SIZE_BYTES 0x400
/**
 * Peripheral base address for regs device on rom_ctrl in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_ROM_CTRL_REGS_BASE_ADDR 0x40007000

/**
 * Peripheral size for regs device on rom_ctrl in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_ROM_CTRL_REGS_BASE_ADDR and
 * `TOP_SAFEROOT_ROM_CTRL_REGS_BASE_ADDR + TOP_SAFEROOT_ROM_CTRL_REGS_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_ROM_CTRL_REGS_SIZE_BYTES 0x80
/**
 * Peripheral base address for regs device on sram_ctrl in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_SRAM_CTRL_REGS_BASE_ADDR 0x40008000

/**
 * Peripheral size for regs device on sram_ctrl in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_SRAM_CTRL_REGS_BASE_ADDR and
 * `TOP_SAFEROOT_SRAM_CTRL_REGS_BASE_ADDR + TOP_SAFEROOT_SRAM_CTRL_REGS_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_SRAM_CTRL_REGS_SIZE_BYTES 0x40
/**
 * Peripheral base address for core device on flash_ctrl in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_FLASH_CTRL_CORE_BASE_ADDR 0x40009000

/**
 * Peripheral size for core device on flash_ctrl in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_FLASH_CTRL_CORE_BASE_ADDR and
 * `TOP_SAFEROOT_FLASH_CTRL_CORE_BASE_ADDR + TOP_SAFEROOT_FLASH_CTRL_CORE_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_FLASH_CTRL_CORE_SIZE_BYTES 0x200
/**
 * Peripheral base address for prim device on flash_ctrl in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_FLASH_CTRL_PRIM_BASE_ADDR 0x4000A000

/**
 * Peripheral size for prim device on flash_ctrl in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_FLASH_CTRL_PRIM_BASE_ADDR and
 * `TOP_SAFEROOT_FLASH_CTRL_PRIM_BASE_ADDR + TOP_SAFEROOT_FLASH_CTRL_PRIM_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_FLASH_CTRL_PRIM_SIZE_BYTES 0x80
/**
 * Peripheral base address for cfg device on rv_core_ibex in top saferoot.
 *
 * This should be used with #mmio_region_from_addr to access the memory-mapped
 * registers associated with the peripheral (usually via a DIF).
 */
#define TOP_SAFEROOT_RV_CORE_IBEX_CFG_BASE_ADDR 0x4000B000

/**
 * Peripheral size for cfg device on rv_core_ibex in top saferoot.
 *
 * This is the size (in bytes) of the peripheral's reserved memory area. All
 * memory-mapped registers associated with this peripheral should have an
 * address between #TOP_SAFEROOT_RV_CORE_IBEX_CFG_BASE_ADDR and
 * `TOP_SAFEROOT_RV_CORE_IBEX_CFG_BASE_ADDR + TOP_SAFEROOT_RV_CORE_IBEX_CFG_SIZE_BYTES`.
 */
#define TOP_SAFEROOT_RV_CORE_IBEX_CFG_SIZE_BYTES 0x100

/**
 * MMIO Region
 *
 * MMIO region for SafeRoot. Excludes ROM, SRAM, and flash data memory.
 */
#define TOP_SAFEROOT_MMIO_BASE_ADDR 0x40000000
#define TOP_SAFEROOT_MMIO_SIZE_BYTES 0x10000000

#endif  // __ASSEMBLER__

#endif  // OPENTITAN_HW_TOP_SAFEROOT_SW_AUTOGEN_TOP_SAFEROOT_MEMORY_H_
