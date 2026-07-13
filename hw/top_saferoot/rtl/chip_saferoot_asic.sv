// Copyright SafeRoot contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// SafeRoot ASIC Chip-Level Wrapper
// This is the top-level module for the SafeRoot ASIC, connecting:
//   - QFN-32 pads (SPI, GPIO, JTAG, power, crystal)
//   - Padring (I/O cells with ESD)
//   - top_saferoot SoC core
//   - Clock/reset generation (simplified -- no AST)
//
// This file replaces the Earl Grey chip-level wrapper which required AST.
// SafeRoot uses a simpler analog front-end without AST module.

module chip_saferoot_asic #(
  parameter RomInitFile = ""  // VMEM file for ROM initialization
) (
  // ====== QFN-32 Pads ======
  // Power (directly bonded, no logic)
  // Pin 1-2:   VDD_CORE (1.8V)
  // Pin 3:     VSS
  // Pin 4-5:   VDD_IO (3.3V)
  // Pin 6:     VSS
  // Pin 26:    VSS
  // Pin 29:    VSS
  // Pin 30-31: VDD_CORE
  // Exposed pad: VSS (thermal)

  // -- SPI Slave (Pin 7-10) --
  input  wire       SPI_CSN,     // Pin 7  -- chip select (active low)
  input  wire       SPI_CLK,     // Pin 8  -- SPI clock
  input  wire       SPI_MOSI,    // Pin 9  -- data in (host -> SafeRoot)
  output wire       SPI_MISO,    // Pin 10 -- data out (SafeRoot -> host)

  // -- Control GPIO (Pin 11-15) --
  output wire       BOOT_OK,     // Pin 11 -- boot chain passed
  output wire       RESET_N,     // Pin 12 -- hard reset host (open-drain)
  output wire       ALERT_N,     // Pin 13 -- security alert (open-drain)
  input  wire       WAKE,        // Pin 14 -- host heartbeat / wake
  output wire       FAILSAFE,    // Pin 15 -- ESTOP / RTH

  // -- General GPIO (Pin 16-19) --
  inout  wire       GPIO_0,      // Pin 16
  inout  wire       GPIO_1,      // Pin 17
  inout  wire       GPIO_2,      // Pin 18 -- alt: UART TX (debug)
  inout  wire       GPIO_3,      // Pin 19 -- alt: UART RX (debug)

  // -- I2C (Pin 20-21) --
  inout  wire       I2C_SDA,     // Pin 20 -- I2C data (optional)
  inout  wire       I2C_SCL,     // Pin 21 -- I2C clock (optional)

  // -- JTAG (Pin 22-25, 32) -- locked in PROD lifecycle --
  input  wire       JTAG_TMS,    // Pin 22
  input  wire       JTAG_TCK,    // Pin 23
  input  wire       JTAG_TDI,    // Pin 24
  output wire       JTAG_TDO,    // Pin 25
  input  wire       JTAG_TRST_N, // Pin 32

  // -- Crystal (Pin 27-28) --
  input  wire       XIN,         // Pin 27 -- crystal oscillator input
  output wire       XOUT         // Pin 28 -- crystal oscillator output
);

  // ================================================================
  // Internal signals
  // ================================================================

  // Clocks
  wire clk_main;   // 100 MHz (from PLL or crystal)
  wire clk_io;     // 96 MHz (derived)
  wire clk_aon;    // 200 kHz (internal RC oscillator)

  // Resets
  wire [1:0] por_n; // Power-on reset (active low)

  // Scan (tied off for production -- no scan in SafeRoot ASIC)
  wire scan_rst_n = 1'b1;
  wire scan_en    = 1'b0;

  // MIO pad connections. top_saferoot exposes 16 MIO (mio_*_i/o [15:0]); the QFN-32
  // product bonds only MIO[7:0] (see mapping below). MIO[15:8] are not bonded: their
  // inputs are tied low and their outputs/oe left unconnected.
  wire [15:0] mio_in;
  wire [15:0] mio_out;
  wire [15:0] mio_oe;
  assign mio_in[15:8] = 8'b0; // unbonded MIO inputs

  // DIO pad connections (directly mapped)
  wire [5:0] dio_in;
  wire [5:0] dio_out;
  wire [5:0] dio_oe;

  // ================================================================
  // Clock Generation (simplified -- no AST)
  // ================================================================
  // In a real ASIC, this would be:
  //   - XIN/XOUT -> PLL -> clk_main (100 MHz)
  //   - Internal divider -> clk_io (96 MHz)
  //   - Internal RC oscillator -> clk_aon (200 kHz, always-on)
  //
  // For simulation/FPGA, connect clk_main directly to XIN.
  assign clk_main = XIN;
  assign clk_io   = XIN;  // TODO: PLL or divider in ASIC
  assign clk_aon  = XIN;  // TODO: RC oscillator in ASIC

  // Power-on reset from external POR circuit
  // In ASIC: voltage detector generates POR
  // For simulation: assume both POR lines active
  assign por_n = 2'b11; // TODO: connect to real POR circuit

  // ================================================================
  // DIO Mapping -- Dedicated I/O pads
  // ================================================================
  // Per top_saferoot_pkg.sv:
  //   DioSpiDeviceSd0 = 0  (MOSI -- data in from host)
  //   DioSpiDeviceSd1 = 1  (MISO -- data out to host)
  //   DioSpiDeviceSd2 = 2  (unused in single SPI mode)
  //   DioSpiDeviceSd3 = 3  (unused in single SPI mode)
  //   DioSpiDeviceSck = 4  (SPI clock)
  //   DioSpiDeviceCsb = 5  (chip select, active low)
  assign dio_in[0] = SPI_MOSI;                              // SD0 = MOSI
  assign SPI_MISO  = dio_out[1];                             // SD1 = MISO (push-pull, not tristate)
  assign dio_in[1] = 1'b0;                                   // SD1 input unused (output only)
  assign dio_in[2] = 1'b0;                                   // SD2 unused
  assign dio_in[3] = 1'b0;                                   // SD3 unused
  assign dio_in[4] = SPI_CLK;                                // SCK
  assign dio_in[5] = SPI_CSN;                                // CSB

  // ================================================================
  // MIO Mapping -- Multiplexed I/O pads
  // ================================================================
  // MIO[0] = GPIO_0 (Pin 16)
  // MIO[1] = GPIO_1 (Pin 17)
  // MIO[2] = GPIO_2 / UART_TX (Pin 18)
  // MIO[3] = GPIO_3 / UART_RX (Pin 19)
  // MIO[4] = BOOT_OK (Pin 11) -- output only
  // MIO[5] = RESET_N (Pin 12) -- output only, open-drain
  // MIO[6] = ALERT_N (Pin 13) -- output only, open-drain
  // MIO[7] = WAKE    (Pin 14) -- input only

  // GPIO bidirectional pads
  // Note: In real ASIC, bidirectional I/O uses padring cells.
  // For Verilator simulation, tristate (1'bz) produces X.
  // Use conditional output instead -- input always reads the pad.
`ifdef VERILATOR
  // Note: no tristate -- output directly drives, input reads
  assign mio_in[0] = GPIO_0;
  assign mio_in[1] = GPIO_1;
  assign mio_in[2] = GPIO_2;
  assign mio_in[3] = GPIO_3;
  // GPIO outputs would need external loopback for simulation
`else
  // Synthesis/ASIC: proper tristate via pad cells
  assign mio_in[0] = GPIO_0;
  assign GPIO_0    = mio_oe[0] ? mio_out[0] : 1'bz;

  assign mio_in[1] = GPIO_1;
  assign GPIO_1    = mio_oe[1] ? mio_out[1] : 1'bz;

  assign mio_in[2] = GPIO_2;
  assign GPIO_2    = mio_oe[2] ? mio_out[2] : 1'bz;

  assign mio_in[3] = GPIO_3;
  assign GPIO_3    = mio_oe[3] ? mio_out[3] : 1'bz;
`endif

  // Control outputs (directly driven)
  assign BOOT_OK   = mio_out[4];
  assign mio_in[4] = 1'b0; // output only

`ifdef VERILATOR
  assign RESET_N   = mio_out[5]; // Note: direct drive
  assign ALERT_N   = mio_out[6];
`else
  assign RESET_N   = mio_oe[5] ? mio_out[5] : 1'bz; // open-drain
  assign ALERT_N   = mio_oe[6] ? mio_out[6] : 1'bz; // open-drain
`endif
  assign mio_in[5] = 1'b0;
  assign mio_in[6] = 1'b0;

  assign mio_in[7] = WAKE; // input only

  // FAILSAFE: driven by alert_handler escalation reset output
  // In top_saferoot.sv, escalation signals go through rstmgr.
  // For now, FAILSAFE mirrors RESET_N (both trigger on escalation).
  // TODO: connect to dedicated esc_rst_tx signal from alert_handler
  assign FAILSAFE  = RESET_N;

  // ================================================================
  // JTAG Connections
  // ================================================================
  // JTAG di qua pinmux ben trong top_saferoot. Pinmux nhan tin hieu
  // JTAG tu MIO pads thong qua strap sampling tai boot time.
  //
  // Trong SafeRoot QFN-32, JTAG physical pins (22-25, 32) SHARE
  // MIO[0:3] qua chip-level mux. Khi dft_strap_test_o active hoac
  // LC state != PROD, JTAG override GPIO tren MIO[0:3].
  //
  // PROD lifecycle: JTAG auto-locked boi lc_ctrl -> MIO[0:3] = GPIO.
  // DEV/RMA lifecycle: JTAG active -> MIO[0:3] = JTAG (GPIO unavailable).

  // DFT strap detection signal tu top_saferoot
  wire jtag_active;
  // Trong ASIC thuc: jtag_active duoc dieu khien boi pinmux strap
  // sampling + LC state. O day dung JTAG_TRST_N active low de detect.
  // Khi TRST_N = 0 (JTAG reset asserted), JTAG mode inactive.
  assign jtag_active = JTAG_TRST_N; // 1 = JTAG active, 0 = GPIO mode

  // Chip-level MIO input mux: JTAG override khi active
  // MIO[0] = JTAG_TMS (khi jtag_active) hoac GPIO_0
  // MIO[1] = JTAG_TCK (khi jtag_active) hoac GPIO_1
  // MIO[2] = JTAG_TDI (khi jtag_active) hoac GPIO_2
  // MIO[3] = GPIO_3 (JTAG TDO la output, khong can MIO input)

  // Override mio_in[0:2] khi JTAG active
  // Luu y: Day la simple mux -- trong ASIC thuc se dung pad mux cells
`ifndef VERILATOR
  // Synthesis: JTAG/GPIO mux tren MIO[0:2] input
  wire [2:0] mio_in_gpio = {GPIO_2, GPIO_1, GPIO_0};
  wire [2:0] mio_in_jtag = {JTAG_TDI, JTAG_TCK, JTAG_TMS};
  // Override global mio_in[0:2] -- can rewire tu phan GPIO o tren
  // (xem phan GPIO bidirectional pads -- da assign mio_in[0:2])
  // Trong ASIC padring thuc, mux nam truoc pad -> khong conflict.
  // O RTL simulation level, assign cuoi cung wins.
`endif

  // JTAG TDO output: route tu MIO output qua pinmux
  // pinmux ben trong top_saferoot se drive mio_out khi JTAG active
  assign JTAG_TDO = mio_out[3]; // MIO[3] output = TDO khi JTAG active

  // ================================================================
  // I2C Connections
  // ================================================================
  // I2C_SDA/SCL share MIO[0:1] qua pinmux alt function.
  // Firmware cau hinh pinmux registers de route I2C signals.
  // Khi I2C active: MIO[0] = I2C_SDA, MIO[1] = I2C_SCL.
  // Khi I2C inactive: MIO[0:1] = GPIO[0:1] (default).
  //
  // Chip-level: I2C physical pins noi song song voi GPIO[0:1].
  // Pinmux ben trong chon source -- khong can mux o chip level.
`ifndef VERILATOR
  // I2C open-drain bidirectional (share pad voi GPIO_0/GPIO_1)
  // Pull-up external required cho I2C
  assign I2C_SDA = mio_oe[0] ? mio_out[0] : 1'bz;
  assign I2C_SCL = mio_oe[1] ? mio_out[1] : 1'bz;
`else
  // Note: I2C outputs tied off (I2C testing can testbench rieng)
  assign I2C_SDA = 1'b1; // pull-up default
  assign I2C_SCL = 1'b1; // pull-up default
`endif

  // ================================================================
  // SoC Core Instance
  // ================================================================
  top_saferoot #(
    .SecRomCtrlDisableScrambling(1),  // Disable ROM scrambling for simulation
`ifdef VERILATOR
    .RvCoreIbexICache(0),             // Disable ICache for Verilator (avoids OTP key dependency)
    .RvCoreIbexICacheScramble(0),
`endif
    .RomCtrlBootRomInitFile(RomInitFile)
  ) u_top (
    // MIO pads
    .mio_in_i  (mio_in),
    .mio_out_o (mio_out),
    .mio_oe_o  (mio_oe),

    // DIO pads
    .dio_in_i  (dio_in),
    .dio_out_o (dio_out),
    .dio_oe_o  (dio_oe),

    // Pad attributes (directly to padring in ASIC -- unused in simulation)
    .mio_attr_o (),
    .dio_attr_o (),

    // RAM/ROM config (tie to defaults)
    .ram_1p_cfg_i     ('0),
    .sram_ctrl_cfg_i  ('0),
    .spi_ram_2p_cfg_i ('0),
    .rom_cfg_i        ('0),

    // Clock bypass/jitter (simplified -- no AST)
    .clk_main_jitter_en_o (),
    .io_clk_byp_req_o     (),
    .io_clk_byp_ack_i     (prim_mubi_pkg::MuBi4False),
    .all_clk_byp_req_o    (),
    .all_clk_byp_ack_i    (prim_mubi_pkg::MuBi4False),
    .hi_speed_sel_o        (),
    .div_step_down_req_i   (prim_mubi_pkg::MuBi4False),
    .calib_rdy_i           (prim_mubi_pkg::MuBi4True),

    // Flash (simplified)
    .flash_bist_enable_i   (prim_mubi_pkg::MuBi4False),
    .flash_power_down_h_i  (1'b0),
    .flash_power_ready_h_i (1'b1),
    .flash_test_mode_a_io  (),
    .flash_test_voltage_h_io (),
    .flash_obs_o           (),

    // Entropy source (simplified -- internal)
    .es_rng_enable_o  (),
    .es_rng_valid_i   (1'b1),
    .es_rng_bit_i     (4'hA), // TODO: connect to real entropy source
    .es_rng_fips_o    (),

    // DFT straps
    .dft_strap_test_o   (),
    .dft_hold_tap_sel_i (1'b0),

    // Power manager (simplified -- no AST)
    .pwrmgr_ast_req_o       (),
    .pwrmgr_ast_rsp_i       (pwrmgr_pkg::PWR_AST_RSP_DEFAULT),

    // OTP macro power sequencing
    .otp_macro_pwr_seq_o    (),
    .otp_macro_pwr_seq_h_i  ('0),
    .otp_ext_voltage_h_io   (),
    .otp_obs_o              (),

    // POR
    .por_n_i (por_n),

    // FPGA info (not used in ASIC)
    .fpga_info_i (32'h0),

    // Sensor controller (simplified -- no AST)
    .sensor_ctrl_ast_alert_req_i ('{default: '0}),
    .sensor_ctrl_ast_alert_rsp_o (),
    .sensor_ctrl_ast_status_i    ('{default: '0}),
    .ast2pinmux_i                (9'b0),
    .ast_init_done_i             (prim_mubi_pkg::MuBi4True),
    .sensor_ctrl_manual_pad_attr_o (),
    .sck_monitor_o               (),

    // Clocks
    .clk_main_i (clk_main),
    .clk_io_i   (clk_io),
    .clk_aon_i  (clk_aon),

    // Clock/reset forwarding to AST (unused -- no AST in SafeRoot)
    .clks_ast_o (),
    .rsts_ast_o (),

    // Scan (disabled in production)
    .scan_rst_ni (scan_rst_n),
    .scan_en_i   (scan_en),
    .scanmode_i  (prim_mubi_pkg::MuBi4False)
  );

  // ================================================================
  // JTAG connections
  // ================================================================
  // JTAG is directly connected through pinmux inside top_saferoot.
  // The pins are exposed here for physical connection.
  // In PROD lifecycle, JTAG is automatically locked by lc_ctrl.
  //
  // Note: JTAG signals are directly routed through the pinmux
  // inside top_saferoot, mapped via the pinmux configuration.
  // No additional logic needed here.

  // ================================================================
  // I2C connections
  // ================================================================
  // I2C SDA/SCL are open-drain, directly connected through pinmux.
  // No additional logic needed here.

  // ================================================================
  // Crystal oscillator
  // ================================================================
  // XOUT is driven by the on-chip oscillator circuit.
  // In simulation, XOUT is unused.
  assign XOUT = 1'b0; // TODO: connect to crystal driver in ASIC

endmodule : chip_saferoot_asic
