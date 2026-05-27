// SafeRoot KMAC (SHA-3 / cSHAKE) Functional Test
// Runs on Ibex CPU, drives KMAC HW accelerator via direct register access
// Output: UART 115200 baud — PASS/FAIL for each test
//
// No HAL driver for KMAC yet — uses REG32_WRITE/REG32_READ directly.
// Register map based on OpenTitan kmac.hjson.

#include <stdint.h>
#include "../hal/hal.h"
#include "../hal/uart_drv.h"

// ============================================================
// KMAC register offsets (base 0x40020000)
// ============================================================
#define KMAC_BASE           0x40020000U

#define KMAC_INTR_STATE     (KMAC_BASE + 0x00)
#define KMAC_INTR_ENABLE    (KMAC_BASE + 0x04)
#define KMAC_CFG_SHADOWED   (KMAC_BASE + 0x10)
#define KMAC_CMD            (KMAC_BASE + 0x14)
#define KMAC_STATUS         (KMAC_BASE + 0x18)
#define KMAC_ENTROPY_PERIOD (KMAC_BASE + 0x1C)
#define KMAC_KEY_LEN        (KMAC_BASE + 0x28)
#define KMAC_KEY_SHARE0     (KMAC_BASE + 0x30)  // 16 words
#define KMAC_KEY_SHARE1     (KMAC_BASE + 0x70)  // 16 words
#define KMAC_MSG_FIFO       (KMAC_BASE + 0xB0)
#define KMAC_STATE           (KMAC_BASE + 0xC0)  // 16 words output

// CMD register commands
#define KMAC_CMD_START      0x1D
#define KMAC_CMD_PROCESS    0x2E
#define KMAC_CMD_DONE       0x3C

// CFG_SHADOWED fields
#define KMAC_CFG_SHA3       0x00   // mode: SHA-3
#define KMAC_CFG_SHAKE      0x02   // mode: SHAKE
#define KMAC_CFG_CSHAKE     0x04   // mode: cSHAKE
#define KMAC_CFG_KMAC       0x06   // mode: KMAC
#define KMAC_CFG_STRENGTH_256 (0x2 << 4)  // strength: 256-bit
#define KMAC_CFG_STRENGTH_128 (0x0 << 4)  // strength: 128-bit
#define KMAC_CFG_MSG_ENDIAN  (1 << 8)     // message byte swap
#define KMAC_CFG_STATE_ENDIAN (1 << 9)    // state byte swap

// STATUS register fields
#define KMAC_STATUS_SHA3_IDLE     BIT(0)
#define KMAC_STATUS_SHA3_ABSORB   BIT(1)
#define KMAC_STATUS_SHA3_SQUEEZE  BIT(2)
#define KMAC_STATUS_FIFO_EMPTY    BIT(14)

// ============================================================
// Helper: print uint32_t[8] as hex
// ============================================================
static void print_state(const uint32_t *state, int words) {
    for (int i = 0; i < words; i++) {
        uart_put_hex(state[i]);
    }
}

// ============================================================
// Helper: busy-wait until KMAC status has expected bits
// ============================================================
static int kmac_wait_status(uint32_t mask, uint32_t expected) {
    for (uint32_t i = 0; i < HAL_TIMEOUT_CYCLES; i++) {
        uint32_t st = REG32_READ(KMAC_STATUS);
        if ((st & mask) == expected)
            return 1;
    }
    return 0;  // timeout
}

// ============================================================
// Test 1: SHA-3-256 of empty string
// Expected: a7ffc6f8 bf1ed766 51c14756 a061d662
//           f580ff4d e43b49fa 82d80a4b 80f8434a
// ============================================================
static const uint32_t sha3_256_empty_expected[8] = {
    0xa7ffc6f8, 0xbf1ed766, 0x51c14756, 0xa061d662,
    0xf580ff4d, 0xe43b49fa, 0x82d80a4b, 0x80f8434a
};

static int test_sha3_256_empty(void) {
    uint32_t state[8];

    uart_puts("[TEST1] SHA-3-256 of empty string\r\n");
    uart_puts("  Expected: ");
    print_state(sha3_256_empty_expected, 8);
    uart_puts("\r\n");

    // Wait for KMAC to be idle
    if (!kmac_wait_status(KMAC_STATUS_SHA3_IDLE, KMAC_STATUS_SHA3_IDLE)) {
        uart_puts("  FAIL: KMAC not idle at start\r\n");
        return 0;
    }

    // Configure: SHA-3, 256-bit strength, endian swap for output
    uint32_t cfg = KMAC_CFG_SHA3 | KMAC_CFG_STRENGTH_256 | KMAC_CFG_STATE_ENDIAN;
    REG32_WRITE(KMAC_CFG_SHADOWED, cfg);
    REG32_WRITE(KMAC_CFG_SHADOWED, cfg);  // shadowed: write twice

    // Issue START command
    REG32_WRITE(KMAC_CMD, KMAC_CMD_START);

    // Wait for absorb state
    if (!kmac_wait_status(KMAC_STATUS_SHA3_ABSORB, KMAC_STATUS_SHA3_ABSORB)) {
        uart_puts("  FAIL: KMAC did not enter absorb state\r\n");
        return 0;
    }

    // No data to write (empty string) — go straight to PROCESS
    REG32_WRITE(KMAC_CMD, KMAC_CMD_PROCESS);

    // Wait for squeeze state (hash complete)
    if (!kmac_wait_status(KMAC_STATUS_SHA3_SQUEEZE, KMAC_STATUS_SHA3_SQUEEZE)) {
        uart_puts("  FAIL: KMAC did not enter squeeze state\r\n");
        return 0;
    }

    // Read first 8 words (256 bits) of state output
    for (int i = 0; i < 8; i++) {
        state[i] = REG32_READ(KMAC_STATE + i * 4);
    }

    // Issue DONE command to return to idle
    REG32_WRITE(KMAC_CMD, KMAC_CMD_DONE);

    uart_puts("  Got:      ");
    print_state(state, 8);
    uart_puts("\r\n");

    // Compare
    uint32_t diff = 0;
    for (int i = 0; i < 8; i++)
        diff |= state[i] ^ sha3_256_empty_expected[i];

    if (diff == 0) {
        uart_puts("  [PASS] SHA-3-256 empty string matches\r\n");
        return 1;
    } else {
        uart_puts("  [FAIL] SHA-3-256 digest mismatch!\r\n");
        uart_puts("  Note: Check endian config if words are byte-swapped\r\n");
        return 0;
    }
}

// ============================================================
// Test 2: KMAC128 with key — verify non-zero output
// Key: 32 bytes of 0x01
// Data: 4 bytes of 0xAA
// No standard test vector — just verify HW produces output
// ============================================================
static int test_kmac128_nonzero(void) {
    uint32_t state[8];

    uart_puts("[TEST2] KMAC128 with key — non-zero output check\r\n");

    // Wait for idle
    if (!kmac_wait_status(KMAC_STATUS_SHA3_IDLE, KMAC_STATUS_SHA3_IDLE)) {
        uart_puts("  FAIL: KMAC not idle at start\r\n");
        return 0;
    }

    // Configure: KMAC mode, 128-bit strength
    uint32_t cfg = KMAC_CFG_KMAC | KMAC_CFG_STRENGTH_128 | KMAC_CFG_STATE_ENDIAN;
    REG32_WRITE(KMAC_CFG_SHADOWED, cfg);
    REG32_WRITE(KMAC_CFG_SHADOWED, cfg);

    // Write 128-bit key into KEY_SHARE0 (4 words), zero KEY_SHARE1
    for (int i = 0; i < 4; i++) {
        REG32_WRITE(KMAC_KEY_SHARE0 + i * 4, 0x01010101);
    }
    for (int i = 4; i < 16; i++) {
        REG32_WRITE(KMAC_KEY_SHARE0 + i * 4, 0x00000000);
    }
    for (int i = 0; i < 16; i++) {
        REG32_WRITE(KMAC_KEY_SHARE1 + i * 4, 0x00000000);
    }

    // Set key length (128-bit = 0)
    REG32_WRITE(KMAC_KEY_LEN, 0x00);

    // Issue START
    REG32_WRITE(KMAC_CMD, KMAC_CMD_START);

    if (!kmac_wait_status(KMAC_STATUS_SHA3_ABSORB, KMAC_STATUS_SHA3_ABSORB)) {
        uart_puts("  FAIL: KMAC did not enter absorb state\r\n");
        return 0;
    }

    // Write 4 bytes of test data
    REG32_WRITE(KMAC_MSG_FIFO, 0xAAAAAAAA);

    // Process
    REG32_WRITE(KMAC_CMD, KMAC_CMD_PROCESS);

    if (!kmac_wait_status(KMAC_STATUS_SHA3_SQUEEZE, KMAC_STATUS_SHA3_SQUEEZE)) {
        uart_puts("  FAIL: KMAC did not enter squeeze state\r\n");
        return 0;
    }

    // Read output state
    uint32_t all_zero = 0;
    for (int i = 0; i < 8; i++) {
        state[i] = REG32_READ(KMAC_STATE + i * 4);
        all_zero |= state[i];
    }

    REG32_WRITE(KMAC_CMD, KMAC_CMD_DONE);

    uart_puts("  Output:   ");
    print_state(state, 8);
    uart_puts("\r\n");

    if (all_zero != 0) {
        uart_puts("  [PASS] KMAC128 produced non-zero output\r\n");
        return 1;
    } else {
        uart_puts("  [FAIL] KMAC128 output is all zeros!\r\n");
        return 0;
    }
}

// ============================================================
// Main
// ============================================================
void main(void) {
    uart_init(115200, 100000000);

    uart_puts("\r\n");
    uart_puts("========================================\r\n");
    uart_puts("  SafeRoot KMAC (SHA-3) Functional Test\r\n");
    uart_puts("  SHA-3-256 + KMAC128\r\n");
    uart_puts("========================================\r\n");
    uart_puts("\r\n");

    int pass = 0, fail = 0;

    if (test_sha3_256_empty()) pass++; else fail++;
    uart_puts("\r\n");

    if (test_kmac128_nonzero()) pass++; else fail++;

    uart_puts("\r\n");
    uart_puts("========================================\r\n");
    uart_puts("  Results: ");
    uart_put_hex((uint32_t)pass);
    uart_puts(" PASS, ");
    uart_put_hex((uint32_t)fail);
    uart_puts(" FAIL\r\n");

    if (fail == 0)
        uart_puts("  [ALL PASS] KMAC HW accelerator OK\r\n");
    else
        uart_puts("  [SOME FAIL] Check KMAC configuration\r\n");

    uart_puts("========================================\r\n");

    while (1) __asm__ volatile("wfi");
}
