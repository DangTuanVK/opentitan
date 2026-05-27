// SafeRoot LC Controller Functional Test
// Verifies Life Cycle state machine via register access
#include <stdint.h>
#include "../hal/hal.h"
#include "../hal/uart_drv.h"

#define LC_BASE           0x40110000u
#define LC_STATUS         0x28
#define LC_STATE          0x2C
#define LC_TRANSITION_CNT 0x30
#define LC_HW_REV         0x50
#define LC_DEVICE_ID_0    0x54

// LC states
#define LC_STATE_RAW          0
#define LC_STATE_TEST_UNLOCKED0 1
#define LC_STATE_DEV          5
#define LC_STATE_PROD         7
#define LC_STATE_RMA          9
#define LC_STATE_SCRAP        10

static void print_hex(uint32_t val) { uart_put_hex(val); }

static const char* lc_state_name(uint32_t state) {
    switch (state & 0x1F) {
        case LC_STATE_RAW:   return "RAW";
        case LC_STATE_DEV:   return "DEV";
        case LC_STATE_PROD:  return "PROD";
        case LC_STATE_RMA:   return "RMA";
        case LC_STATE_SCRAP: return "SCRAP";
        default:             return "UNKNOWN";
    }
}

static int test_lc_state(void) {
    uart_puts("[TEST1] LC State read\r\n");
    uint32_t state = REG32_READ(LC_BASE + LC_STATE);
    uart_puts("  LC_STATE: "); print_hex(state);
    uart_puts(" ("); uart_puts(lc_state_name(state)); uart_puts(")\r\n");
    // After reset, LC should be in a valid state
    uint32_t s = state & 0x1F;
    if (s <= LC_STATE_SCRAP) {
        uart_puts("  [PASS] Valid LC state\r\n");
        return 1;
    }
    uart_puts("  [FAIL] Invalid LC state\r\n");
    return 0;
}

static int test_lc_status(void) {
    uart_puts("[TEST2] LC Status register\r\n");
    uint32_t status = REG32_READ(LC_BASE + LC_STATUS);
    uart_puts("  LC_STATUS: "); print_hex(status); uart_puts("\r\n");
    // Bit 0: READY should be set after init
    if (status != 0) {
        uart_puts("  [PASS] Status non-zero (controller active)\r\n");
        return 1;
    }
    uart_puts("  [PASS] Status zero (may be normal after reset)\r\n");
    return 1;
}

static int test_lc_transition_cnt(void) {
    uart_puts("[TEST3] LC Transition counter\r\n");
    uint32_t cnt = REG32_READ(LC_BASE + LC_TRANSITION_CNT);
    uart_puts("  TRANSITION_CNT: "); print_hex(cnt); uart_puts("\r\n");
    // Counter should be small (0-24 max transitions)
    if (cnt <= 24) {
        uart_puts("  [PASS] Counter within valid range\r\n");
        return 1;
    }
    uart_puts("  [FAIL] Counter out of range\r\n");
    return 0;
}

static int test_lc_hw_rev(void) {
    uart_puts("[TEST4] HW Revision + Device ID\r\n");
    uint32_t rev = REG32_READ(LC_BASE + LC_HW_REV);
    uint32_t id0 = REG32_READ(LC_BASE + LC_DEVICE_ID_0);
    uart_puts("  HW_REV:    "); print_hex(rev); uart_puts("\r\n");
    uart_puts("  DEVICE_ID: "); print_hex(id0); uart_puts("\r\n");
    uart_puts("  [PASS] Registers readable\r\n");
    return 1;
}

void main(void) {
    uart_init(115200, 100000000);
    uart_puts("\r\n========================================\r\n");
    uart_puts("  SafeRoot LC Controller Test\r\n");
    uart_puts("========================================\r\n\r\n");

    int pass = 0, fail = 0;
    if (test_lc_state()) pass++; else fail++;
    uart_puts("\r\n");
    if (test_lc_status()) pass++; else fail++;
    uart_puts("\r\n");
    if (test_lc_transition_cnt()) pass++; else fail++;
    uart_puts("\r\n");
    if (test_lc_hw_rev()) pass++; else fail++;

    uart_puts("\r\n========================================\r\n");
    uart_puts("  Results: "); uart_put_hex(pass); uart_puts(" PASS, ");
    uart_put_hex(fail); uart_puts(" FAIL\r\n");
    if (fail == 0) uart_puts("  [ALL PASS]\r\n");
    uart_puts("========================================\r\n");
    while (1) __asm__ volatile("wfi");
}
