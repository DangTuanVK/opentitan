// SafeRoot CPU + IP Test — Verilator simulation
// Monitors UART TX output via bit-bang decoder
#include "Vchip_saferoot_tb.h"
#include "Vchip_saferoot_tb___024root.h"
#include "verilated.h"
//#include "verilated_vcd_c.h"
#include <iostream>
#include <cstdint>
#include <string>

// UART TX bit-bang decoder
// 115200 baud @ 100MHz = 868 cycles per bit
class UartDecoder {
    static const int BAUD = 868;
    int state, bit_cnt, sample_cnt;
    uint8_t rx_byte;
    int prev;
public:
    int total_chars;
    std::string output;

    UartDecoder() : state(0), bit_cnt(0), sample_cnt(0),
                    rx_byte(0), prev(1), total_chars(0) {}

    void tick(int tx) {
        switch (state) {
        case 0: // idle
            if (prev == 1 && tx == 0) {
                state = 1;
                sample_cnt = BAUD / 2;
            }
            break;
        case 1: // start bit middle
            if (--sample_cnt <= 0) {
                if (tx == 0) {
                    state = 2; bit_cnt = 0; rx_byte = 0;
                    sample_cnt = BAUD;
                } else state = 0;
            }
            break;
        case 2: // data bits
            if (--sample_cnt <= 0) {
                rx_byte |= (tx << bit_cnt);
                if (++bit_cnt >= 8) state = 3;
                sample_cnt = BAUD;
            }
            break;
        case 3: // stop bit
            if (--sample_cnt <= 0) {
                state = 0;
                total_chars++;
                char c = (char)rx_byte;
                if (c >= 32 || c == '\n' || c == '\r')
                    output += c;
                // Print char immediately
                if (c == '\n')
                    std::cout << std::endl;
                else if (c >= 32)
                    std::cout << c;
                std::cout.flush();
            }
            break;
        }
        prev = tx;
    }
};

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    Vchip_saferoot_tb *top = new Vchip_saferoot_tb;

    std::cout << "--- SafeRoot Verilator Simulation ---" << std::endl;
    std::cout << "UART TX output:" << std::endl;
    std::cout << std::endl;

    UartDecoder uart;

    // 2000 timesteps = 1000 cycles for quick debug
    const vluint64_t MAX = 2000;
    vluint64_t t = 0;

    while (!Verilated::gotFinish() && t < MAX) {
        top->clk_ext = !top->clk_ext;
        top->eval();
        //tfp->dump(t);

        // Sample UART on rising edge
        if (top->clk_ext) {
            uart.tick(top->uart_tx_o);

            // Debug: print signals every 1K cycles (first 10K), then every 50K
            vluint64_t cyc = t/2;
            bool dbg = (cyc >= 39 && cyc <= 200) || (cyc % 50000 == 0);
            if (dbg) {
                fprintf(stderr, "[DBG %7lu] PC=0x%08x tx=%d dbus=%d d_out=%d\n",
                    (unsigned long)cyc, top->cpu_pc_o,
                    (int)top->uart_tx_o,
                    (int)top->dbg_dbus_valid_o,
                    (int)top->rootp->chip_saferoot_tb__DOT__u_dut__DOT__u_top__DOT__u_xbar_main__DOT__u_s1n_38__DOT__dev_select_outstanding);
            }
        }

        t++;
    }

    std::cout << std::endl;
    std::cout << "--- Simulation End ---" << std::endl;
    std::cout << "Cycles: " << t/2 << std::endl;
    std::cout << "UART chars: " << uart.total_chars << std::endl;

    if (uart.output.find("PASS") != std::string::npos)
        std::cout << "[RESULT] Tests detected: PASS found in output" << std::endl;
    if (uart.output.find("FAIL") != std::string::npos)
        std::cout << "[RESULT] WARNING: FAIL found in output" << std::endl;
    if (uart.total_chars == 0)
        std::cout << "[RESULT] No UART output — CPU may not have booted" << std::endl;

    delete top;
    return (uart.output.find("FAIL") != std::string::npos) ? 1 : 0;
}
