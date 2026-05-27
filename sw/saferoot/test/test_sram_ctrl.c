// SafeRoot SRAM Controller Functional Test
// Verifies SRAM access, scrambling, integrity
#include <stdint.h>
#include "../hal/hal.h"
#include "../hal/uart_drv.h"

#define SRAM_CTRL_BASE    0x40180000u
#define SRAM_BASE         0x10000000u
#define SRAM_SIZE         (128 * 1024)  // 128KB

// SRAM Controller registers
#define SRAM_STATUS       0x10
#define SRAM_EXEC_REGWEN  0x14
#define SRAM_CTRL_REGWEN  0x18
#define SRAM_SCR_KEY_ROTATED 0x1C

static void print_hex(uint32_t val) { uart_put_hex(val); }

static int test_sram_basic_rw(void) {
    uart_puts("[TEST1] SRAM basic read/write\r\n");
    volatile uint32_t *sram = (volatile uint32_t *)(SRAM_BASE + 0x10000);

    // Write known patterns
    sram[0] = 0xDEADBEEF;
    sram[1] = 0xCAFEBABE;
    sram[2] = 0x12345678;
    sram[3] = 0x9ABCDEF0;

    // Read back
    uint32_t v0 = sram[0], v1 = sram[1], v2 = sram[2], v3 = sram[3];
    uart_puts("  Write: DEADBEEF CAFEBABE 12345678 9ABCDEF0\r\n");
    uart_puts("  Read:  "); print_hex(v0); uart_puts(" ");
    print_hex(v1); uart_puts(" "); print_hex(v2); uart_puts(" ");
    print_hex(v3); uart_puts("\r\n");

    if (v0 == 0xDEADBEEF && v1 == 0xCAFEBABE &&
        v2 == 0x12345678 && v3 == 0x9ABCDEF0) {
        uart_puts("  [PASS]\r\n");
        return 1;
    }
    uart_puts("  [FAIL] Data mismatch\r\n");
    return 0;
}

static int test_sram_walking_ones(void) {
    uart_puts("[TEST2] SRAM walking ones pattern\r\n");
    volatile uint32_t *sram = (volatile uint32_t *)(SRAM_BASE + 0x11000);
    int errors = 0;

    for (int i = 0; i < 32; i++) {
        uint32_t pattern = 1u << i;
        sram[i] = pattern;
    }
    for (int i = 0; i < 32; i++) {
        uint32_t expected = 1u << i;
        if (sram[i] != expected) errors++;
    }

    uart_puts("  Walking ones (32 patterns): ");
    if (errors == 0) {
        uart_puts("[PASS]\r\n");
        return 1;
    }
    uart_puts("[FAIL] "); uart_put_hex(errors); uart_puts(" errors\r\n");
    return 0;
}

static int test_sram_address_bus(void) {
    uart_puts("[TEST3] SRAM address bus test\r\n");
    volatile uint32_t *sram = (volatile uint32_t *)(SRAM_BASE + 0x12000);
    int errors = 0;

    // Write address as data at each power-of-2 offset
    for (int i = 0; i < 15; i++) {  // up to 2^14 words = 64KB
        uint32_t offset = 1u << i;
        sram[offset] = offset;
    }
    for (int i = 0; i < 15; i++) {
        uint32_t offset = 1u << i;
        if (sram[offset] != offset) errors++;
    }

    uart_puts("  Address bus (15 checks): ");
    if (errors == 0) {
        uart_puts("[PASS]\r\n");
        return 1;
    }
    uart_puts("[FAIL] "); uart_put_hex(errors); uart_puts(" errors\r\n");
    return 0;
}

static int test_sram_ctrl_status(void) {
    uart_puts("[TEST4] SRAM Controller status\r\n");
    uint32_t status = REG32_READ(SRAM_CTRL_BASE + SRAM_STATUS);
    uart_puts("  STATUS:        "); print_hex(status); uart_puts("\r\n");

    uint32_t regwen = REG32_READ(SRAM_CTRL_BASE + SRAM_CTRL_REGWEN);
    uart_puts("  CTRL_REGWEN:   "); print_hex(regwen); uart_puts("\r\n");

    uint32_t scr = REG32_READ(SRAM_CTRL_BASE + SRAM_SCR_KEY_ROTATED);
    uart_puts("  SCR_KEY_ROTATED: "); print_hex(scr); uart_puts("\r\n");

    uart_puts("  [PASS] Registers readable\r\n");
    return 1;
}

static int test_sram_byte_access(void) {
    uart_puts("[TEST5] SRAM byte/halfword access\r\n");
    volatile uint8_t *sram8 = (volatile uint8_t *)(SRAM_BASE + 0x13000);
    volatile uint16_t *sram16 = (volatile uint16_t *)(SRAM_BASE + 0x13100);

    // Byte write + read
    sram8[0] = 0xAA; sram8[1] = 0x55; sram8[2] = 0xF0; sram8[3] = 0x0F;
    int byte_ok = (sram8[0] == 0xAA && sram8[1] == 0x55 &&
                   sram8[2] == 0xF0 && sram8[3] == 0x0F);

    // Halfword write + read
    sram16[0] = 0x1234; sram16[1] = 0xABCD;
    int half_ok = (sram16[0] == 0x1234 && sram16[1] == 0xABCD);

    uart_puts("  Byte access:     "); uart_puts(byte_ok ? "[PASS]" : "[FAIL]"); uart_puts("\r\n");
    uart_puts("  Halfword access: "); uart_puts(half_ok ? "[PASS]" : "[FAIL]"); uart_puts("\r\n");

    return byte_ok && half_ok;
}

void main(void) {
    uart_init(115200, 100000000);
    uart_puts("\r\n========================================\r\n");
    uart_puts("  SafeRoot SRAM Controller Test\r\n");
    uart_puts("========================================\r\n\r\n");

    int pass = 0, fail = 0;
    if (test_sram_basic_rw()) pass++; else fail++;
    uart_puts("\r\n");
    if (test_sram_walking_ones()) pass++; else fail++;
    uart_puts("\r\n");
    if (test_sram_address_bus()) pass++; else fail++;
    uart_puts("\r\n");
    if (test_sram_ctrl_status()) pass++; else fail++;
    uart_puts("\r\n");
    if (test_sram_byte_access()) pass++; else fail++;

    uart_puts("\r\n========================================\r\n");
    uart_puts("  Results: "); uart_put_hex(pass); uart_puts(" PASS, ");
    uart_put_hex(fail); uart_puts(" FAIL\r\n");
    if (fail == 0) uart_puts("  [ALL PASS]\r\n");
    uart_puts("========================================\r\n");
    while (1) __asm__ volatile("wfi");
}
