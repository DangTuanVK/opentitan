// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// ------------------- W A R N I N G: A U T O - G E N E R A T E D   C O D E !! -------------------//
// PLEASE DO NOT HAND-EDIT THIS FILE. IT HAS BEEN AUTO-GENERATED WITH THE FOLLOWING COMMAND:
//
// util/topgen.py -t hw/top_saferoot/data/top_saferoot.hjson
//                -o hw/top_saferoot/

package top_saferoot_pkg;
  /**
   * Peripheral base address for aes in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_AES_BASE_ADDR = 32'h40000000;

  /**
   * Peripheral size in bytes for aes in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_AES_SIZE_BYTES = 32'h100;

  /**
   * Peripheral base address for hmac in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_HMAC_BASE_ADDR = 32'h40010000;

  /**
   * Peripheral size in bytes for hmac in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_HMAC_SIZE_BYTES = 32'h2000;

  /**
   * Peripheral base address for kmac in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_KMAC_BASE_ADDR = 32'h40020000;

  /**
   * Peripheral size in bytes for kmac in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_KMAC_SIZE_BYTES = 32'h1000;

  /**
   * Peripheral base address for csrng in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_CSRNG_BASE_ADDR = 32'h40030000;

  /**
   * Peripheral size in bytes for csrng in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_CSRNG_SIZE_BYTES = 32'h80;

  /**
   * Peripheral base address for entropy_src in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_ENTROPY_SRC_BASE_ADDR = 32'h40040000;

  /**
   * Peripheral size in bytes for entropy_src in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_ENTROPY_SRC_SIZE_BYTES = 32'h100;

  /**
   * Peripheral base address for edn0 in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_EDN0_BASE_ADDR = 32'h40005000;

  /**
   * Peripheral size in bytes for edn0 in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_EDN0_SIZE_BYTES = 32'h80;

  /**
   * Peripheral base address for otbn in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_OTBN_BASE_ADDR = 32'h40080000;

  /**
   * Peripheral size in bytes for otbn in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_OTBN_SIZE_BYTES = 32'h10000;

  /**
   * Peripheral base address for keymgr in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_KEYMGR_BASE_ADDR = 32'h40100000;

  /**
   * Peripheral size in bytes for keymgr in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_KEYMGR_SIZE_BYTES = 32'h100;

  /**
   * Peripheral base address for regs device on lc_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_LC_CTRL_REGS_BASE_ADDR = 32'h40101000;

  /**
   * Peripheral size in bytes for regs device on lc_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_LC_CTRL_REGS_SIZE_BYTES = 32'h100;

  /**
   * Peripheral base address for dmi device on lc_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_LC_CTRL_DMI_BASE_ADDR = 32'h0;

  /**
   * Peripheral size in bytes for dmi device on lc_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_LC_CTRL_DMI_SIZE_BYTES = 32'h1000;

  /**
   * Peripheral base address for alert_handler in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_ALERT_HANDLER_BASE_ADDR = 32'h40120000;

  /**
   * Peripheral size in bytes for alert_handler in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_ALERT_HANDLER_SIZE_BYTES = 32'h800;

  /**
   * Peripheral base address for core device on otp_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_OTP_CTRL_CORE_BASE_ADDR = 32'h40130000;

  /**
   * Peripheral size in bytes for core device on otp_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_OTP_CTRL_CORE_SIZE_BYTES = 32'h1000;

  /**
   * Peripheral base address for prim device on otp_macro in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_OTP_MACRO_PRIM_BASE_ADDR = 32'h40138000;

  /**
   * Peripheral size in bytes for prim device on otp_macro in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_OTP_MACRO_PRIM_SIZE_BYTES = 32'h20;

  /**
   * Peripheral base address for sensor_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_SENSOR_CTRL_BASE_ADDR = 32'h40140000;

  /**
   * Peripheral size in bytes for sensor_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_SENSOR_CTRL_SIZE_BYTES = 32'h80;

  /**
   * Peripheral base address for spi_device in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_SPI_DEVICE_BASE_ADDR = 32'h40200000;

  /**
   * Peripheral size in bytes for spi_device in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_SPI_DEVICE_SIZE_BYTES = 32'h2000;

  /**
   * Peripheral base address for gpio in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_GPIO_BASE_ADDR = 32'h40210000;

  /**
   * Peripheral size in bytes for gpio in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_GPIO_SIZE_BYTES = 32'h80;

  /**
   * Peripheral base address for i2c0 in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_I2C0_BASE_ADDR = 32'h40220000;

  /**
   * Peripheral size in bytes for i2c0 in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_I2C0_SIZE_BYTES = 32'h80;

  /**
   * Peripheral base address for uart0 in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_UART0_BASE_ADDR = 32'h40230000;

  /**
   * Peripheral size in bytes for uart0 in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_UART0_SIZE_BYTES = 32'h40;

  /**
   * Peripheral base address for rv_plic in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_RV_PLIC_BASE_ADDR = 32'h48000000;

  /**
   * Peripheral size in bytes for rv_plic in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_RV_PLIC_SIZE_BYTES = 32'h8000000;

  /**
   * Peripheral base address for rv_timer in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_RV_TIMER_BASE_ADDR = 32'h40310000;

  /**
   * Peripheral size in bytes for rv_timer in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_RV_TIMER_SIZE_BYTES = 32'h200;

  /**
   * Peripheral base address for aon_timer_aon in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_AON_TIMER_AON_BASE_ADDR = 32'h40320000;

  /**
   * Peripheral size in bytes for aon_timer_aon in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_AON_TIMER_AON_SIZE_BYTES = 32'h40;

  /**
   * Peripheral base address for pwrmgr_aon in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_PWRMGR_AON_BASE_ADDR = 32'h40330000;

  /**
   * Peripheral size in bytes for pwrmgr_aon in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_PWRMGR_AON_SIZE_BYTES = 32'h80;

  /**
   * Peripheral base address for clkmgr_aon in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_CLKMGR_AON_BASE_ADDR = 32'h40340000;

  /**
   * Peripheral size in bytes for clkmgr_aon in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_CLKMGR_AON_SIZE_BYTES = 32'h40;

  /**
   * Peripheral base address for rstmgr_aon in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_RSTMGR_AON_BASE_ADDR = 32'h40350000;

  /**
   * Peripheral size in bytes for rstmgr_aon in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_RSTMGR_AON_SIZE_BYTES = 32'h40;

  /**
   * Peripheral base address for pinmux_aon in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_PINMUX_AON_BASE_ADDR = 32'h40360000;

  /**
   * Peripheral size in bytes for pinmux_aon in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_PINMUX_AON_SIZE_BYTES = 32'h400;

  /**
   * Peripheral base address for regs device on rom_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_ROM_CTRL_REGS_BASE_ADDR = 32'h40007000;

  /**
   * Peripheral size in bytes for regs device on rom_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_ROM_CTRL_REGS_SIZE_BYTES = 32'h80;

  /**
   * Peripheral base address for regs device on sram_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_SRAM_CTRL_REGS_BASE_ADDR = 32'h40008000;

  /**
   * Peripheral size in bytes for regs device on sram_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_SRAM_CTRL_REGS_SIZE_BYTES = 32'h40;

  /**
   * Peripheral base address for core device on flash_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_FLASH_CTRL_CORE_BASE_ADDR = 32'h40009000;

  /**
   * Peripheral size in bytes for core device on flash_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_FLASH_CTRL_CORE_SIZE_BYTES = 32'h200;

  /**
   * Peripheral base address for prim device on flash_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_FLASH_CTRL_PRIM_BASE_ADDR = 32'h4000A000;

  /**
   * Peripheral size in bytes for prim device on flash_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_FLASH_CTRL_PRIM_SIZE_BYTES = 32'h80;

  /**
   * Peripheral base address for cfg device on rv_core_ibex in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_RV_CORE_IBEX_CFG_BASE_ADDR = 32'h4000B000;

  /**
   * Peripheral size in bytes for cfg device on rv_core_ibex in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_RV_CORE_IBEX_CFG_SIZE_BYTES = 32'h100;

  /**
   * Memory base address for rom memory on rom_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_ROM_CTRL_ROM_BASE_ADDR = 32'h0;

  /**
   * Memory size for rom memory on rom_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_ROM_CTRL_ROM_SIZE_BYTES = 32'h8000;

  /**
   * Memory base address for ram memory on sram_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_SRAM_CTRL_RAM_BASE_ADDR = 32'h10000000;

  /**
   * Memory size for ram memory on sram_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_SRAM_CTRL_RAM_SIZE_BYTES = 32'h20000;

  /**
   * Memory base address for mem memory on flash_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_FLASH_CTRL_MEM_BASE_ADDR = 32'h20000000;

  /**
   * Memory size for mem memory on flash_ctrl in top saferoot.
   */
  parameter int unsigned TOP_SAFEROOT_FLASH_CTRL_MEM_SIZE_BYTES = 32'h100000;


  // Enumeration of alert modules
  typedef enum int unsigned {
    TopSaferootAlertPeripheralAes = 0,
    TopSaferootAlertPeripheralHmac = 1,
    TopSaferootAlertPeripheralKmac = 2,
    TopSaferootAlertPeripheralCsrng = 3,
    TopSaferootAlertPeripheralEntropySrc = 4,
    TopSaferootAlertPeripheralEdn0 = 5,
    TopSaferootAlertPeripheralOtbn = 6,
    TopSaferootAlertPeripheralKeymgr = 7,
    TopSaferootAlertPeripheralLcCtrl = 8,
    TopSaferootAlertPeripheralOtpCtrl = 9,
    TopSaferootAlertPeripheralSensorCtrl = 10,
    TopSaferootAlertPeripheralSpiDevice = 11,
    TopSaferootAlertPeripheralGpio = 12,
    TopSaferootAlertPeripheralI2c0 = 13,
    TopSaferootAlertPeripheralUart0 = 14,
    TopSaferootAlertPeripheralRvPlic = 15,
    TopSaferootAlertPeripheralRvTimer = 16,
    TopSaferootAlertPeripheralAonTimerAon = 17,
    TopSaferootAlertPeripheralPwrmgrAon = 18,
    TopSaferootAlertPeripheralClkmgrAon = 19,
    TopSaferootAlertPeripheralRstmgrAon = 20,
    TopSaferootAlertPeripheralPinmuxAon = 21,
    TopSaferootAlertPeripheralRomCtrl = 22,
    TopSaferootAlertPeripheralSramCtrl = 23,
    TopSaferootAlertPeripheralFlashCtrl = 24,
    TopSaferootAlertPeripheralRvCoreIbex = 25,
    TopSaferootAlertPeripheralCount
  } alert_peripheral_e;

  // Enumeration of alerts
  typedef enum int unsigned {
    TopSaferootAlertIdAesRecovCtrlUpdateErr = 0,
    TopSaferootAlertIdAesFatalFault = 1,
    TopSaferootAlertIdHmacFatalFault = 2,
    TopSaferootAlertIdKmacRecovOperationErr = 3,
    TopSaferootAlertIdKmacFatalFaultErr = 4,
    TopSaferootAlertIdCsrngRecovAlert = 5,
    TopSaferootAlertIdCsrngFatalAlert = 6,
    TopSaferootAlertIdEntropySrcRecovAlert = 7,
    TopSaferootAlertIdEntropySrcFatalAlert = 8,
    TopSaferootAlertIdEdn0RecovAlert = 9,
    TopSaferootAlertIdEdn0FatalAlert = 10,
    TopSaferootAlertIdOtbnFatal = 11,
    TopSaferootAlertIdOtbnRecov = 12,
    TopSaferootAlertIdKeymgrRecovOperationErr = 13,
    TopSaferootAlertIdKeymgrFatalFaultErr = 14,
    TopSaferootAlertIdLcCtrlFatalProgError = 15,
    TopSaferootAlertIdLcCtrlFatalStateError = 16,
    TopSaferootAlertIdLcCtrlFatalBusIntegError = 17,
    TopSaferootAlertIdOtpCtrlFatalMacroError = 18,
    TopSaferootAlertIdOtpCtrlFatalCheckError = 19,
    TopSaferootAlertIdOtpCtrlFatalBusIntegError = 20,
    TopSaferootAlertIdOtpCtrlFatalPrimOtpAlert = 21,
    TopSaferootAlertIdOtpCtrlRecovPrimOtpAlert = 22,
    TopSaferootAlertIdSensorCtrlRecovAlert = 23,
    TopSaferootAlertIdSensorCtrlFatalAlert = 24,
    TopSaferootAlertIdSpiDeviceFatalFault = 25,
    TopSaferootAlertIdGpioFatalFault = 26,
    TopSaferootAlertIdI2c0FatalFault = 27,
    TopSaferootAlertIdUart0FatalFault = 28,
    TopSaferootAlertIdRvPlicFatalFault = 29,
    TopSaferootAlertIdRvTimerFatalFault = 30,
    TopSaferootAlertIdAonTimerAonFatalFault = 31,
    TopSaferootAlertIdPwrmgrAonFatalFault = 32,
    TopSaferootAlertIdClkmgrAonRecovFault = 33,
    TopSaferootAlertIdClkmgrAonFatalFault = 34,
    TopSaferootAlertIdRstmgrAonFatalFault = 35,
    TopSaferootAlertIdRstmgrAonFatalCnstyFault = 36,
    TopSaferootAlertIdPinmuxAonFatalFault = 37,
    TopSaferootAlertIdRomCtrlFatal = 38,
    TopSaferootAlertIdSramCtrlFatalError = 39,
    TopSaferootAlertIdFlashCtrlRecovErr = 40,
    TopSaferootAlertIdFlashCtrlFatalStdErr = 41,
    TopSaferootAlertIdFlashCtrlFatalErr = 42,
    TopSaferootAlertIdFlashCtrlFatalPrimFlashAlert = 43,
    TopSaferootAlertIdFlashCtrlRecovPrimFlashAlert = 44,
    TopSaferootAlertIdRvCoreIbexFatalSwErr = 45,
    TopSaferootAlertIdRvCoreIbexRecovSwErr = 46,
    TopSaferootAlertIdRvCoreIbexFatalHwErr = 47,
    TopSaferootAlertIdRvCoreIbexRecovHwErr = 48,
    TopSaferootAlertIdCount
  } alert_id_e;

  // Enumeration of interrupts
  typedef enum int unsigned {
    TopSaferootPlicIrqIdNone = 0,
    TopSaferootPlicIrqIdHmacHmacDone = 1,
    TopSaferootPlicIrqIdHmacFifoEmpty = 2,
    TopSaferootPlicIrqIdHmacHmacErr = 3,
    TopSaferootPlicIrqIdKmacKmacDone = 4,
    TopSaferootPlicIrqIdKmacFifoEmpty = 5,
    TopSaferootPlicIrqIdKmacKmacErr = 6,
    TopSaferootPlicIrqIdCsrngCsCmdReqDone = 7,
    TopSaferootPlicIrqIdCsrngCsEntropyReq = 8,
    TopSaferootPlicIrqIdCsrngCsHwInstExc = 9,
    TopSaferootPlicIrqIdCsrngCsFatalErr = 10,
    TopSaferootPlicIrqIdEntropySrcEsEntropyValid = 11,
    TopSaferootPlicIrqIdEntropySrcEsHealthTestFailed = 12,
    TopSaferootPlicIrqIdEntropySrcEsObserveFifoReady = 13,
    TopSaferootPlicIrqIdEntropySrcEsFatalErr = 14,
    TopSaferootPlicIrqIdEdn0EdnCmdReqDone = 15,
    TopSaferootPlicIrqIdEdn0EdnFatalErr = 16,
    TopSaferootPlicIrqIdOtbnDone = 17,
    TopSaferootPlicIrqIdKeymgrOpDone = 18,
    TopSaferootPlicIrqIdAlertHandlerClassa = 19,
    TopSaferootPlicIrqIdAlertHandlerClassb = 20,
    TopSaferootPlicIrqIdAlertHandlerClassc = 21,
    TopSaferootPlicIrqIdAlertHandlerClassd = 22,
    TopSaferootPlicIrqIdOtpCtrlOtpOperationDone = 23,
    TopSaferootPlicIrqIdOtpCtrlOtpError = 24,
    TopSaferootPlicIrqIdSensorCtrlIoStatusChange = 25,
    TopSaferootPlicIrqIdSensorCtrlInitStatusChange = 26,
    TopSaferootPlicIrqIdSpiDeviceUploadCmdfifoNotEmpty = 27,
    TopSaferootPlicIrqIdSpiDeviceUploadPayloadNotEmpty = 28,
    TopSaferootPlicIrqIdSpiDeviceUploadPayloadOverflow = 29,
    TopSaferootPlicIrqIdSpiDeviceReadbufWatermark = 30,
    TopSaferootPlicIrqIdSpiDeviceReadbufFlip = 31,
    TopSaferootPlicIrqIdSpiDeviceTpmHeaderNotEmpty = 32,
    TopSaferootPlicIrqIdSpiDeviceTpmRdfifoCmdEnd = 33,
    TopSaferootPlicIrqIdSpiDeviceTpmRdfifoDrop = 34,
    TopSaferootPlicIrqIdGpioGpio0 = 35,
    TopSaferootPlicIrqIdGpioGpio1 = 36,
    TopSaferootPlicIrqIdGpioGpio2 = 37,
    TopSaferootPlicIrqIdGpioGpio3 = 38,
    TopSaferootPlicIrqIdGpioGpio4 = 39,
    TopSaferootPlicIrqIdGpioGpio5 = 40,
    TopSaferootPlicIrqIdGpioGpio6 = 41,
    TopSaferootPlicIrqIdGpioGpio7 = 42,
    TopSaferootPlicIrqIdGpioGpio8 = 43,
    TopSaferootPlicIrqIdGpioGpio9 = 44,
    TopSaferootPlicIrqIdGpioGpio10 = 45,
    TopSaferootPlicIrqIdGpioGpio11 = 46,
    TopSaferootPlicIrqIdGpioGpio12 = 47,
    TopSaferootPlicIrqIdGpioGpio13 = 48,
    TopSaferootPlicIrqIdGpioGpio14 = 49,
    TopSaferootPlicIrqIdGpioGpio15 = 50,
    TopSaferootPlicIrqIdGpioGpio16 = 51,
    TopSaferootPlicIrqIdGpioGpio17 = 52,
    TopSaferootPlicIrqIdGpioGpio18 = 53,
    TopSaferootPlicIrqIdGpioGpio19 = 54,
    TopSaferootPlicIrqIdGpioGpio20 = 55,
    TopSaferootPlicIrqIdGpioGpio21 = 56,
    TopSaferootPlicIrqIdGpioGpio22 = 57,
    TopSaferootPlicIrqIdGpioGpio23 = 58,
    TopSaferootPlicIrqIdGpioGpio24 = 59,
    TopSaferootPlicIrqIdGpioGpio25 = 60,
    TopSaferootPlicIrqIdGpioGpio26 = 61,
    TopSaferootPlicIrqIdGpioGpio27 = 62,
    TopSaferootPlicIrqIdGpioGpio28 = 63,
    TopSaferootPlicIrqIdGpioGpio29 = 64,
    TopSaferootPlicIrqIdGpioGpio30 = 65,
    TopSaferootPlicIrqIdGpioGpio31 = 66,
    TopSaferootPlicIrqIdI2c0FmtThreshold = 67,
    TopSaferootPlicIrqIdI2c0RxThreshold = 68,
    TopSaferootPlicIrqIdI2c0AcqThreshold = 69,
    TopSaferootPlicIrqIdI2c0RxOverflow = 70,
    TopSaferootPlicIrqIdI2c0ControllerHalt = 71,
    TopSaferootPlicIrqIdI2c0SclInterference = 72,
    TopSaferootPlicIrqIdI2c0SdaInterference = 73,
    TopSaferootPlicIrqIdI2c0StretchTimeout = 74,
    TopSaferootPlicIrqIdI2c0SdaUnstable = 75,
    TopSaferootPlicIrqIdI2c0CmdComplete = 76,
    TopSaferootPlicIrqIdI2c0TxStretch = 77,
    TopSaferootPlicIrqIdI2c0TxThreshold = 78,
    TopSaferootPlicIrqIdI2c0AcqStretch = 79,
    TopSaferootPlicIrqIdI2c0UnexpStop = 80,
    TopSaferootPlicIrqIdI2c0HostTimeout = 81,
    TopSaferootPlicIrqIdUart0TxWatermark = 82,
    TopSaferootPlicIrqIdUart0RxWatermark = 83,
    TopSaferootPlicIrqIdUart0TxDone = 84,
    TopSaferootPlicIrqIdUart0RxOverflow = 85,
    TopSaferootPlicIrqIdUart0RxFrameErr = 86,
    TopSaferootPlicIrqIdUart0RxBreakErr = 87,
    TopSaferootPlicIrqIdUart0RxTimeout = 88,
    TopSaferootPlicIrqIdUart0RxParityErr = 89,
    TopSaferootPlicIrqIdUart0TxEmpty = 90,
    TopSaferootPlicIrqIdRvTimerTimerExpiredHart0Timer0 = 91,
    TopSaferootPlicIrqIdAonTimerAonWkupTimerExpired = 92,
    TopSaferootPlicIrqIdAonTimerAonWdogTimerBark = 93,
    TopSaferootPlicIrqIdPwrmgrAonWakeup = 94,
    TopSaferootPlicIrqIdFlashCtrlProgEmpty = 95,
    TopSaferootPlicIrqIdFlashCtrlProgLvl = 96,
    TopSaferootPlicIrqIdFlashCtrlRdFull = 97,
    TopSaferootPlicIrqIdFlashCtrlRdLvl = 98,
    TopSaferootPlicIrqIdFlashCtrlOpDone = 99,
    TopSaferootPlicIrqIdFlashCtrlCorrErr = 100,
    TopSaferootPlicIrqIdCount
  } interrupt_rv_plic_id_e;


  // Enumeration of IO power domains.
  // Only used in ASIC target.
  typedef enum logic [0:0] {
    IoBankVcc = 0,
    IoBankCount = 1
  } pwr_dom_e;

  // Enumeration for MIO signals on the top-level.
  typedef enum int unsigned {
    MioInGpioGpio0 = 0,
    MioInGpioGpio1 = 1,
    MioInGpioGpio2 = 2,
    MioInGpioGpio3 = 3,
    MioInGpioGpio4 = 4,
    MioInGpioGpio5 = 5,
    MioInGpioGpio6 = 6,
    MioInGpioGpio7 = 7,
    MioInGpioGpio8 = 8,
    MioInGpioGpio9 = 9,
    MioInGpioGpio10 = 10,
    MioInGpioGpio11 = 11,
    MioInGpioGpio12 = 12,
    MioInGpioGpio13 = 13,
    MioInGpioGpio14 = 14,
    MioInGpioGpio15 = 15,
    MioInGpioGpio16 = 16,
    MioInGpioGpio17 = 17,
    MioInGpioGpio18 = 18,
    MioInGpioGpio19 = 19,
    MioInGpioGpio20 = 20,
    MioInGpioGpio21 = 21,
    MioInGpioGpio22 = 22,
    MioInGpioGpio23 = 23,
    MioInGpioGpio24 = 24,
    MioInGpioGpio25 = 25,
    MioInGpioGpio26 = 26,
    MioInGpioGpio27 = 27,
    MioInGpioGpio28 = 28,
    MioInGpioGpio29 = 29,
    MioInGpioGpio30 = 30,
    MioInGpioGpio31 = 31,
    MioInI2c0Sda = 32,
    MioInI2c0Scl = 33,
    MioInUart0Rx = 34,
    MioInSpiDeviceTpmCsb = 35,
    MioInFlashCtrlTck = 36,
    MioInFlashCtrlTms = 37,
    MioInFlashCtrlTdi = 38,
    MioInCount = 39
  } mio_in_e;

  typedef enum {
    MioOutGpioGpio0 = 0,
    MioOutGpioGpio1 = 1,
    MioOutGpioGpio2 = 2,
    MioOutGpioGpio3 = 3,
    MioOutGpioGpio4 = 4,
    MioOutGpioGpio5 = 5,
    MioOutGpioGpio6 = 6,
    MioOutGpioGpio7 = 7,
    MioOutGpioGpio8 = 8,
    MioOutGpioGpio9 = 9,
    MioOutGpioGpio10 = 10,
    MioOutGpioGpio11 = 11,
    MioOutGpioGpio12 = 12,
    MioOutGpioGpio13 = 13,
    MioOutGpioGpio14 = 14,
    MioOutGpioGpio15 = 15,
    MioOutGpioGpio16 = 16,
    MioOutGpioGpio17 = 17,
    MioOutGpioGpio18 = 18,
    MioOutGpioGpio19 = 19,
    MioOutGpioGpio20 = 20,
    MioOutGpioGpio21 = 21,
    MioOutGpioGpio22 = 22,
    MioOutGpioGpio23 = 23,
    MioOutGpioGpio24 = 24,
    MioOutGpioGpio25 = 25,
    MioOutGpioGpio26 = 26,
    MioOutGpioGpio27 = 27,
    MioOutGpioGpio28 = 28,
    MioOutGpioGpio29 = 29,
    MioOutGpioGpio30 = 30,
    MioOutGpioGpio31 = 31,
    MioOutI2c0Sda = 32,
    MioOutI2c0Scl = 33,
    MioOutUart0Tx = 34,
    MioOutFlashCtrlTdo = 35,
    MioOutSensorCtrlAstDebugOut0 = 36,
    MioOutSensorCtrlAstDebugOut1 = 37,
    MioOutSensorCtrlAstDebugOut2 = 38,
    MioOutSensorCtrlAstDebugOut3 = 39,
    MioOutSensorCtrlAstDebugOut4 = 40,
    MioOutSensorCtrlAstDebugOut5 = 41,
    MioOutSensorCtrlAstDebugOut6 = 42,
    MioOutSensorCtrlAstDebugOut7 = 43,
    MioOutSensorCtrlAstDebugOut8 = 44,
    MioOutOtpMacroTest0 = 45,
    MioOutCount = 46
  } mio_out_e;

  // Enumeration for DIO signals, used on both the top and chip-levels.
  typedef enum int unsigned {
    DioSpiDeviceSd0 = 0,
    DioSpiDeviceSd1 = 1,
    DioSpiDeviceSd2 = 2,
    DioSpiDeviceSd3 = 3,
    DioSpiDeviceSck = 4,
    DioSpiDeviceCsb = 5,
    DioCount = 6
  } dio_e;

  // Enumeration for the types of pads.
  typedef enum {
    MioPad,
    DioPad
  } pad_type_e;

  // Raw MIO/DIO input array indices on chip-level.
  // TODO: Does not account for target specific stubbed/added pads.
  // Need to make a target-specific package for those.
  typedef enum int unsigned {
    MioPadMio0 = 0,
    MioPadMio1 = 1,
    MioPadMio2 = 2,
    MioPadMio3 = 3,
    MioPadMio4 = 4,
    MioPadMio5 = 5,
    MioPadMio6 = 6,
    MioPadMio7 = 7,
    MioPadMio8 = 8,
    MioPadMio9 = 9,
    MioPadMio10 = 10,
    MioPadMio11 = 11,
    MioPadMio12 = 12,
    MioPadMio13 = 13,
    MioPadMio14 = 14,
    MioPadMio15 = 15,
    MioPadCount
  } mio_pad_e;

  typedef enum int unsigned {
    DioPadPorN = 0,
    DioPadOtpExtVolt = 1,
    DioPadSpiDevD0 = 2,
    DioPadSpiDevD1 = 3,
    DioPadSpiDevD2 = 4,
    DioPadSpiDevD3 = 5,
    DioPadSpiDevClk = 6,
    DioPadSpiDevCsL = 7,
    DioPadBootOk = 8,
    DioPadCount
  } dio_pad_e;

  // List of peripheral instantiated in this chip.
  typedef enum {
    PeripheralAes,
    PeripheralAlertHandler,
    PeripheralAonTimerAon,
    PeripheralClkmgrAon,
    PeripheralCsrng,
    PeripheralEdn0,
    PeripheralEntropySrc,
    PeripheralFlashCtrl,
    PeripheralGpio,
    PeripheralHmac,
    PeripheralI2c0,
    PeripheralKeymgr,
    PeripheralKmac,
    PeripheralLcCtrl,
    PeripheralOtbn,
    PeripheralOtpCtrl,
    PeripheralOtpMacro,
    PeripheralPinmuxAon,
    PeripheralPwrmgrAon,
    PeripheralRomCtrl,
    PeripheralRstmgrAon,
    PeripheralRvCoreIbex,
    PeripheralRvPlic,
    PeripheralRvTimer,
    PeripheralSensorCtrl,
    PeripheralSpiDevice,
    PeripheralSramCtrl,
    PeripheralUart0,
    PeripheralCount
  } peripheral_e;

  // MMIO Region
  //
  parameter int unsigned TOP_SAFEROOT_MMIO_BASE_ADDR = 32'h40000000;
  parameter int unsigned TOP_SAFEROOT_MMIO_SIZE_BYTES = 32'h10000000;

  // TODO: Enumeration for PLIC Interrupt source peripheral.

// MACROs for AST analog simulation support
`ifdef ANALOGSIM
  `define INOUT_AI input ast_pkg::awire_t
  `define INOUT_AO output ast_pkg::awire_t
`else
  `define INOUT_AI inout
  `define INOUT_AO inout
`endif

endpackage
