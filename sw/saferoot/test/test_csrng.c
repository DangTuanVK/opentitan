// SafeRoot CSRNG (Cryptographically Secure RNG) Functional Test
// Runs on Ibex CPU, drives CSRNG HW via direct register access
// Output: UART 115200 baud — PASS/FAIL for each test
//
// Tests: read random data, verify non-zero, verify different reads,
// and basic statistical check (bit distribution).

#include <stdint.h>
#include "../hal/hal.h"
#include "../hal/uart_drv.h"

// ============================================================
// CSRNG register offsets (base 0x40030000)
// ============================================================
#define CSRNG_BASE          0x40030000U

#define CSRNG_INTR_STATE    (CSRNG_BASE + 0x00)
#define CSRNG_INTR_ENABLE   (CSRNG_BASE + 0x04)
#define CSRNG_CTRL          (CSRNG_BASE + 0x10)
#define CSRNG_CMD_REQ       (CSRNG_BASE + 0x14)
#define CSRNG_SW_CMD_STS    (CSRNG_BASE + 0x18)
#define CSRNG_GENBITS_VLD   (CSRNG_BASE + 0x1C)
#define CSRNG_GENBITS       (CSRNG_BASE + 0x20)

// CSRNG commands (NIST SP 800-90A style)
// Command format: cmd[3:0] | clen[7:4] | flags[11:8] | glen[30:12]
#define CSRNG_CMD_INSTANTIATE  0x01
#define CSRNG_CMD_GENERATE     0x03
#define CSRNG_CMD_UNINSTANTIATE 0x05

// Generate command: generate 1 block (128 bits = 4 words)
// glen = 1 (1 block of 128 bits), cmd = GENERATE
#define CSRNG_GEN_1BLK  (CSRNG_CMD_GENERATE | (1U << 12))

// CTRL register: enable CSRNG
#define CSRNG_CTRL_ENABLE  0x06  // enable=1, sw_app_enable=1

// ============================================================
// Helper: wait for GENBITS_VLD
// ============================================================
static int csrng_wait_valid(void) {
    for (uint32_t i = 0; i < HAL_TIMEOUT_CYCLES; i++) {
        if (REG32_READ(CSRNG_GENBITS_VLD) & 0x1)
            return 1;
    }
    return 0;
}

// ============================================================
// Helper: wait for command ready (SW_CMD_STS)
// ============================================================
static int csrng_wait_cmd_ready(void) {
    for (uint32_t i = 0; i < HAL_TIMEOUT_CYCLES; i++) {
        uint32_t sts = REG32_READ(CSRNG_SW_CMD_STS);
        if (sts & 0x1)  // cmd_rdy bit
            return 1;
    }
    return 0;
}

// ============================================================
// Initialize CSRNG
// ============================================================
static int csrng_init(void) {
    // Enable CSRNG
    REG32_WRITE(CSRNG_CTRL, CSRNG_CTRL_ENABLE);

    // Wait for command ready
    if (!csrng_wait_cmd_ready()) return 0;

    // Instantiate (seed from entropy source)
    // flag=1 means use internal entropy, clen=0 (no additional data)
    REG32_WRITE(CSRNG_CMD_REQ, CSRNG_CMD_INSTANTIATE | (1U << 8));

    return csrng_wait_cmd_ready();
}

// ============================================================
// Read 4 random words (128 bits) into buf[4]
// ============================================================
static int csrng_generate(uint32_t buf[4]) {
    if (!csrng_wait_cmd_ready()) return 0;

    // Issue generate command for 1 block
    REG32_WRITE(CSRNG_CMD_REQ, CSRNG_GEN_1BLK);

    // Read 4 words of output
    for (int i = 0; i < 4; i++) {
        if (!csrng_wait_valid()) return 0;
        buf[i] = REG32_READ(CSRNG_GENBITS);
    }
    return 1;
}

// ============================================================
// Helper: count set bits (popcount) in a uint32_t
// ============================================================
static int popcount32(uint32_t v) {
    int count = 0;
    while (v) {
        count += v & 1;
        v >>= 1;
    }
    return count;
}

// ============================================================
// Test 1: Read 32 random bytes (8 words), verify non-zero
// ============================================================
static int test_csrng_nonzero(void) {
    uint32_t buf[8];

    uart_puts("[TEST1] CSRNG: read 32 bytes, verify non-zero\r\n");

    // Generate 2 blocks (8 words = 32 bytes)
    if (!csrng_generate(&buf[0])) {
        uart_puts("  FAIL: generate block 0 timeout\r\n");
        return 0;
    }
    if (!csrng_generate(&buf[4])) {
        uart_puts("  FAIL: generate block 1 timeout\r\n");
        return 0;
    }

    uart_puts("  Random: ");
    for (int i = 0; i < 8; i++)
        uart_put_hex(buf[i]);
    uart_puts("\r\n");

    // Check non-zero
    uint32_t all_or = 0;
    for (int i = 0; i < 8; i++)
        all_or |= buf[i];

    if (all_or != 0) {
        uart_puts("  [PASS] Random data is non-zero\r\n");
        return 1;
    } else {
        uart_puts("  [FAIL] All zeros — CSRNG not producing output\r\n");
        return 0;
    }
}

// ============================================================
// Test 2: Read 32 more random bytes, verify different from first
// ============================================================
static uint32_t first_batch[8];
static uint32_t second_batch[8];

static int test_csrng_different(void) {
    uart_puts("[TEST2] CSRNG: two reads should differ\r\n");

    // Read first batch
    if (!csrng_generate(&first_batch[0]) || !csrng_generate(&first_batch[4])) {
        uart_puts("  FAIL: generate first batch timeout\r\n");
        return 0;
    }

    // Read second batch
    if (!csrng_generate(&second_batch[0]) || !csrng_generate(&second_batch[4])) {
        uart_puts("  FAIL: generate second batch timeout\r\n");
        return 0;
    }

    uart_puts("  Batch 1: ");
    for (int i = 0; i < 8; i++) uart_put_hex(first_batch[i]);
    uart_puts("\r\n");
    uart_puts("  Batch 2: ");
    for (int i = 0; i < 8; i++) uart_put_hex(second_batch[i]);
    uart_puts("\r\n");

    // Compare
    uint32_t diff = 0;
    for (int i = 0; i < 8; i++)
        diff |= first_batch[i] ^ second_batch[i];

    if (diff != 0) {
        uart_puts("  [PASS] Two reads produced different values\r\n");
        return 1;
    } else {
        uart_puts("  [FAIL] Both reads identical — CSRNG stuck?\r\n");
        return 0;
    }
}

// ============================================================
// Test 3: Statistical check — bit distribution in 128 bytes
// Count 1-bits, expect ~40%-60% for roughly uniform output
// 128 bytes = 1024 bits, expect ~410-614 ones
// ============================================================
static int test_csrng_stats(void) {
    uint32_t buf[4];
    int total_ones = 0;
    int total_bits = 0;

    uart_puts("[TEST3] CSRNG: statistical bit distribution (128 bytes)\r\n");

    // Generate 128 bytes = 32 words = 8 blocks
    for (int blk = 0; blk < 8; blk++) {
        if (!csrng_generate(buf)) {
            uart_puts("  FAIL: generate timeout at block ");
            uart_put_hex((uint32_t)blk);
            uart_puts("\r\n");
            return 0;
        }
        for (int w = 0; w < 4; w++) {
            total_ones += popcount32(buf[w]);
        }
    }

    total_bits = 128 * 8;  // 1024 bits

    uart_puts("  Total bits: ");
    uart_put_hex((uint32_t)total_bits);
    uart_puts("\r\n");
    uart_puts("  Ones count: ");
    uart_put_hex((uint32_t)total_ones);
    uart_puts("\r\n");

    // 40% of 1024 = 409, 60% of 1024 = 614
    int low  = (total_bits * 40) / 100;   // 409
    int high = (total_bits * 60) / 100;   // 614

    uart_puts("  Range [40%-60%]: ");
    uart_put_hex((uint32_t)low);
    uart_puts(" - ");
    uart_put_hex((uint32_t)high);
    uart_puts("\r\n");

    if (total_ones >= low && total_ones <= high) {
        uart_puts("  [PASS] Bit distribution within expected range\r\n");
        return 1;
    } else {
        uart_puts("  [FAIL] Bit distribution out of range — possible bias\r\n");
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
    uart_puts("  SafeRoot CSRNG Functional Test\r\n");
    uart_puts("  Random number generator checks\r\n");
    uart_puts("========================================\r\n");
    uart_puts("\r\n");

    // Initialize CSRNG
    if (!csrng_init()) {
        uart_puts("FATAL: CSRNG init failed — aborting tests\r\n");
        while (1) __asm__ volatile("wfi");
    }
    uart_puts("CSRNG initialized OK\r\n\r\n");

    int pass = 0, fail = 0;

    if (test_csrng_nonzero()) pass++; else fail++;
    uart_puts("\r\n");

    if (test_csrng_different()) pass++; else fail++;
    uart_puts("\r\n");

    if (test_csrng_stats()) pass++; else fail++;

    uart_puts("\r\n");
    uart_puts("========================================\r\n");
    uart_puts("  Results: ");
    uart_put_hex((uint32_t)pass);
    uart_puts(" PASS, ");
    uart_put_hex((uint32_t)fail);
    uart_puts(" FAIL\r\n");

    if (fail == 0)
        uart_puts("  [ALL PASS] CSRNG HW OK\r\n");
    else
        uart_puts("  [SOME FAIL] Check CSRNG / entropy source\r\n");

    uart_puts("========================================\r\n");

    while (1) __asm__ volatile("wfi");
}
