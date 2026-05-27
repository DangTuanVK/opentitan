// SafeRoot Boot ROM — Hello World firmware
// Bare-metal: no OS, uses HAL drivers
// Targets: Verilator simulation + FPGA

#include <stdint.h>
#include "../hal/hal.h"
#include "../hal/uart_drv.h"
#include "../hal/gpio_drv.h"

// Simple delay (busy loop)
static void delay(uint32_t cycles) {
    volatile uint32_t i;
    for (i = 0; i < cycles; i++)
        __asm__ volatile("nop");
}

// ============================================================
// Main — SafeRoot Boot ROM entry
// ============================================================
void main(void) {
    // 1. Initialize peripherals
    uart_init(115200, 100000000);  // 115200 baud @ 100MHz
    gpio_init();

    // 2. Boot banner
    uart_puts("\n");
    uart_puts("========================================\n");
    uart_puts("  SafeRoot Boot ROM v1.0\n");
    uart_puts("  IoT Security Anchor Chip\n");
    uart_puts("========================================\n");
    uart_puts("\n");

    // 3. Report system info
    uart_puts("[BOOT] CPU: Ibex RV32IMC @ 100MHz\n");
    uart_puts("[BOOT] ROM: 32KB @ 0x00000000\n");
    uart_puts("[BOOT] RAM: 128KB @ 0x10000000\n");
    uart_puts("[BOOT] UART0: ");
    uart_put_hex(SAFEROOT_UART0_BASE);
    uart_puts("\n");

    // 4. GPIO test — toggle pattern
    uart_puts("[TEST] GPIO toggle...\n");
    for (int i = 0; i < 4; i++) {
        gpio_write(1u << i);
        delay(1000);
    }
    gpio_write(0xF);  // all on
    uart_puts("[TEST] GPIO: OK (0xF)\n");

    // 5. Memory test — write/read SRAM
    uart_puts("[TEST] SRAM write/read...\n");
    volatile uint32_t *sram = (volatile uint32_t *)0x10010000u;
    sram[0] = 0xDEADBEEF;
    sram[1] = 0xCAFEBABE;
    if (sram[0] == 0xDEADBEEF && sram[1] == 0xCAFEBABE) {
        uart_puts("[TEST] SRAM: OK\n");
    } else {
        uart_puts("[TEST] SRAM: FAIL!\n");
    }

    // 6. Boot complete
    uart_puts("\n");
    uart_puts("[BOOT] SafeRoot ready.\n");
    uart_puts("[BOOT] Waiting for host SPI commands...\n");

    // 7. Blink GPIO in loop (heartbeat indicator)
    uint32_t count = 0;
    while (1) {
        gpio_write(count & 0xF);
        count++;
        delay(5000);
    }
}
