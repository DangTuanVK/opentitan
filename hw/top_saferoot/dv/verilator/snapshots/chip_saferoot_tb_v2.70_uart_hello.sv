// SafeRoot testbench — direct ROM bypass for Verilator
// Intercepts ibex instruction bus to serve ROM directly

module chip_saferoot_tb (
  input  logic clk_ext,
  output logic uart_tx_o,
  output logic boot_ok_o,
  output logic gpio0_o,
  output logic [31:0] cpu_pc_o,
  output logic dbg_dbus_valid_o
);

  wire SPI_MISO, BOOT_OK, RESET_N, ALERT_N, FAILSAFE;
  wire GPIO_0, GPIO_1, GPIO_2, GPIO_3;
  wire I2C_SDA, I2C_SCL;
  wire JTAG_TDO, XOUT;

  chip_saferoot_asic #(
    .RomInitFile("rom.vmem")
  ) u_dut (
    .SPI_CSN(1'b1), .SPI_CLK(1'b0), .SPI_MOSI(1'b0), .SPI_MISO(SPI_MISO),
    .BOOT_OK(BOOT_OK), .RESET_N(RESET_N), .ALERT_N(ALERT_N),
    .WAKE(1'b0), .FAILSAFE(FAILSAFE),
    .GPIO_0(GPIO_0), .GPIO_1(GPIO_1), .GPIO_2(GPIO_2), .GPIO_3(GPIO_3),
    .I2C_SDA(I2C_SDA), .I2C_SCL(I2C_SCL),
    .JTAG_TMS(1'b0), .JTAG_TCK(1'b0), .JTAG_TDI(1'b0),
    .JTAG_TDO(JTAG_TDO), .JTAG_TRST_N(1'b1),
    .XIN(clk_ext), .XOUT(XOUT)
  );

  // Direct signal taps
  assign uart_tx_o = u_dut.u_top.cio_uart0_tx_d2p;
  assign boot_ok_o = BOOT_OK;
  assign gpio0_o   = GPIO_0;
  assign cpu_pc_o  = u_dut.u_top.u_rv_core_ibex.u_core.u_ibex_core.pc_id;

  // Snoop ibex data bus directly from core signals
  wire        dbg_data_req  = u_dut.u_top.u_rv_core_ibex.main_core_data_req;
  wire [31:0] dbg_data_addr = u_dut.u_top.u_rv_core_ibex.main_core_data_addr;
  wire [31:0] dbg_data_wdata= u_dut.u_top.u_rv_core_ibex.main_core_data_wdata;
  wire        dbg_data_we   = u_dut.u_top.u_rv_core_ibex.main_core_data_we;
  assign dbg_dbus_valid_o = dbg_data_req;

  // Software UART snoop: detect writes to 0x4000001C (UART WDATA)
  always @(posedge clk_ext) begin
    if (dbg_data_req && dbg_data_we && dbg_data_addr == 32'h4000001C) begin
      $write("%c", dbg_data_wdata[7:0]);
    end
  end

  initial $display("[TB] SafeRoot Verilator testbench — ICache disabled, SecureIbex disabled");
endmodule
