// SafeRoot Key Manager (DICE) Functional Test
// Runs on Ibex CPU, drives KeyMgr HW via direct register access
// Output: UART 115200 baud — PASS/FAIL for each test
//
// Tests: check initial state, advance state, read key output.
// Register map based on OpenTitan keymgr.hjson.

#include <stdint.h>
#include "../hal/hal.h"
#include "../hal/uart_drv.h"

// ============================================================
// KeyMgr register offsets (base 0x40100000)
// ============================================================
#define KEYMGR_BASE             0x40100000U

#define KEYMGR_INTR_STATE       (KEYMGR_BASE + 0x00)
#define KEYMGR_INTR_ENABLE      (KEYMGR_BASE + 0x04)
#define KEYMGR_CFG_REGWEN       (KEYMGR_BASE + 0x10)
#define KEYMGR_START             (KEYMGR_BASE + 0x14)
#define KEYMGR_CONTROL          (KEYMGR_BASE + 0x18)
#define KEYMGR_SIDELOAD_CLEAR   (KEYMGR_BASE + 0x1C)
#define KEYMGR_RESEED_INTERVAL  (KEYMGR_BASE + 0x20)
#define KEYMGR_SW_BINDING       (KEYMGR_BASE + 0x28)  // 8 words
#define KEYMGR_SALT             (KEYMGR_BASE + 0x50)   // 8 words
#define KEYMGR_KEY_VERSION      (KEYMGR_BASE + 0x80)
#define KEYMGR_SW_SHARE0_STATUS (KEYMGR_BASE + 0x88)
#define KEYMGR_SW_SHARE0_OUTPUT (KEYMGR_BASE + 0x8C)   // 8 words
#define KEYMGR_SW_SHARE1_OUTPUT (KEYMGR_BASE + 0xB0)   // 8 words
#define KEYMGR_WORKING_STATE    (KEYMGR_BASE + 0xD8)
#define KEYMGR_OP_STATUS        (KEYMGR_BASE + 0xDC)

// WORKING_STATE values
#define KEYMGR_STATE_RESET       0x0
#define KEYMGR_STATE_INIT        0x1
#define KEYMGR_STATE_CREATOR     0x2
#define KEYMGR_STATE_OWNER_INT   0x3
#define KEYMGR_STATE_OWNER       0x4
#define KEYMGR_STATE_DISABLED    0x5
#define KEYMGR_STATE_INVALID     0x6

// CONTROL register fields
#define KEYMGR_CTRL_OP_ADVANCE   (0x0 << 4)  // operation: advance state
#define KEYMGR_CTRL_OP_GENERATE  (0x1 << 4)  // operation: generate key
#define KEYMGR_CTRL_DEST_SW      (0x3 << 12) // destination: SW output registers

// OP_STATUS values
#define KEYMGR_OP_IDLE           0x0
#define KEYMGR_OP_WIP            0x1
#define KEYMGR_OP_DONE_SUCCESS   0x2
#define KEYMGR_OP_DONE_ERROR     0x3

// START register: write 1 to trigger operation
#define KEYMGR_START_EN          0x1

// ============================================================
// Helper: get state name string
// ============================================================
static const char *state_name(uint32_t state) {
    switch (state) {
        case KEYMGR_STATE_RESET:      return "Reset";
        case KEYMGR_STATE_INIT:       return "Init";
        case KEYMGR_STATE_CREATOR:    return "CreatorRootKey";
        case KEYMGR_STATE_OWNER_INT:  return "OwnerIntermediateKey";
        case KEYMGR_STATE_OWNER:      return "OwnerKey";
        case KEYMGR_STATE_DISABLED:   return "Disabled";
        case KEYMGR_STATE_INVALID:    return "Invalid";
        default:                      return "Unknown";
    }
}

// ============================================================
// Helper: wait for operation to complete
// ============================================================
static int keymgr_wait_op_done(void) {
    for (uint32_t i = 0; i < HAL_TIMEOUT_CYCLES; i++) {
        uint32_t st = REG32_READ(KEYMGR_OP_STATUS);
        if (st == KEYMGR_OP_DONE_SUCCESS || st == KEYMGR_OP_DONE_ERROR)
            return (int)st;
    }
    return -1;  // timeout
}

// ============================================================
// Test 1: Read WORKING_STATE — should be Reset (0) or Init (1)
// ============================================================
static int test_keymgr_initial_state(void) {
    uart_puts("[TEST1] KeyMgr: check initial WORKING_STATE\r\n");

    uint32_t state = REG32_READ(KEYMGR_WORKING_STATE);

    uart_puts("  WORKING_STATE: ");
    uart_put_hex(state);
    uart_puts(" (");
    uart_puts(state_name(state));
    uart_puts(")\r\n");

    if (state == KEYMGR_STATE_RESET || state == KEYMGR_STATE_INIT) {
        uart_puts("  [PASS] KeyMgr in expected initial state\r\n");
        return 1;
    } else {
        uart_puts("  [FAIL] KeyMgr not in Reset/Init state after boot\r\n");
        return 0;
    }
}

// ============================================================
// Test 2: Advance keymgr state — verify state changes
// From Reset -> Init (or Init -> CreatorRootKey)
// ============================================================
static int test_keymgr_advance(void) {
    uart_puts("[TEST2] KeyMgr: advance state\r\n");

    uint32_t state_before = REG32_READ(KEYMGR_WORKING_STATE);
    uart_puts("  State before: ");
    uart_put_hex(state_before);
    uart_puts(" (");
    uart_puts(state_name(state_before));
    uart_puts(")\r\n");

    // Check if config register is writable
    uint32_t regwen = REG32_READ(KEYMGR_CFG_REGWEN);
    uart_puts("  CFG_REGWEN:   ");
    uart_put_hex(regwen);
    uart_puts("\r\n");

    if (!(regwen & 0x1)) {
        uart_puts("  WARN: CFG_REGWEN locked, cannot advance\r\n");
        uart_puts("  [SKIP] State advance not possible\r\n");
        return 1;  // Not a failure — may be locked by OTP
    }

    // Set SW binding values (required for advance)
    for (int i = 0; i < 8; i++) {
        REG32_WRITE(KEYMGR_SW_BINDING + i * 4, 0x12345678 + i);
    }

    // Set SALT values
    for (int i = 0; i < 8; i++) {
        REG32_WRITE(KEYMGR_SALT + i * 4, 0xAABBCC00 + i);
    }

    // Set key version
    REG32_WRITE(KEYMGR_KEY_VERSION, 0x00000001);

    // Configure: advance operation
    REG32_WRITE(KEYMGR_CONTROL, KEYMGR_CTRL_OP_ADVANCE);

    // Trigger start
    REG32_WRITE(KEYMGR_START, KEYMGR_START_EN);

    // Wait for completion
    int op_result = keymgr_wait_op_done();

    uint32_t state_after = REG32_READ(KEYMGR_WORKING_STATE);
    uart_puts("  State after:  ");
    uart_put_hex(state_after);
    uart_puts(" (");
    uart_puts(state_name(state_after));
    uart_puts(")\r\n");

    uart_puts("  OP_STATUS:    ");
    uart_put_hex(REG32_READ(KEYMGR_OP_STATUS));
    uart_puts("\r\n");

    if (op_result < 0) {
        uart_puts("  [FAIL] Advance operation timed out\r\n");
        return 0;
    }

    if (state_after != state_before) {
        uart_puts("  [PASS] KeyMgr state advanced successfully\r\n");
        return 1;
    } else if ((uint32_t)op_result == KEYMGR_OP_DONE_ERROR) {
        uart_puts("  [FAIL] Advance returned error (check OTP/LC)\r\n");
        return 0;
    } else {
        uart_puts("  [FAIL] State did not change\r\n");
        return 0;
    }
}

// ============================================================
// Test 3: Read SW_SHARE0_OUTPUT — verify non-zero key material
// Only valid after keymgr has advanced to CreatorRootKey or later
// ============================================================
static int test_keymgr_key_output(void) {
    uint32_t share0[8];
    uint32_t all_or = 0;

    uart_puts("[TEST3] KeyMgr: read SW key output\r\n");

    uint32_t state = REG32_READ(KEYMGR_WORKING_STATE);
    uart_puts("  Current state: ");
    uart_puts(state_name(state));
    uart_puts("\r\n");

    // If we're still in Reset/Init, we need to generate first
    if (state >= KEYMGR_STATE_CREATOR) {
        // Configure: generate SW key output
        REG32_WRITE(KEYMGR_CONTROL, KEYMGR_CTRL_OP_GENERATE | KEYMGR_CTRL_DEST_SW);
        REG32_WRITE(KEYMGR_START, KEYMGR_START_EN);

        int op_result = keymgr_wait_op_done();
        if (op_result < 0) {
            uart_puts("  FAIL: generate operation timed out\r\n");
            return 0;
        }
        if ((uint32_t)op_result == KEYMGR_OP_DONE_ERROR) {
            uart_puts("  FAIL: generate returned error\r\n");
            return 0;
        }
    }

    // Read SW_SHARE0_OUTPUT (8 words)
    uart_puts("  SW_SHARE0: ");
    for (int i = 0; i < 8; i++) {
        share0[i] = REG32_READ(KEYMGR_SW_SHARE0_OUTPUT + i * 4);
        uart_put_hex(share0[i]);
        all_or |= share0[i];
    }
    uart_puts("\r\n");

    // Also read SW_SHARE1_OUTPUT for completeness
    uart_puts("  SW_SHARE1: ");
    for (int i = 0; i < 8; i++) {
        uint32_t w = REG32_READ(KEYMGR_SW_SHARE1_OUTPUT + i * 4);
        uart_put_hex(w);
        all_or |= w;
    }
    uart_puts("\r\n");

    if (all_or != 0) {
        uart_puts("  [PASS] Key material is non-zero\r\n");
        return 1;
    } else {
        uart_puts("  [FAIL] Key output is all zeros\r\n");
        uart_puts("  Note: KeyMgr may need to be in CreatorRootKey+ state\r\n");
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
    uart_puts("  SafeRoot Key Manager (DICE) Test\r\n");
    uart_puts("  State machine + key generation\r\n");
    uart_puts("========================================\r\n");
    uart_puts("\r\n");

    int pass = 0, fail = 0;

    if (test_keymgr_initial_state()) pass++; else fail++;
    uart_puts("\r\n");

    if (test_keymgr_advance()) pass++; else fail++;
    uart_puts("\r\n");

    if (test_keymgr_key_output()) pass++; else fail++;

    uart_puts("\r\n");
    uart_puts("========================================\r\n");
    uart_puts("  Results: ");
    uart_put_hex((uint32_t)pass);
    uart_puts(" PASS, ");
    uart_put_hex((uint32_t)fail);
    uart_puts(" FAIL\r\n");

    if (fail == 0)
        uart_puts("  [ALL PASS] Key Manager HW OK\r\n");
    else
        uart_puts("  [SOME FAIL] Check KeyMgr / OTP / LC state\r\n");

    uart_puts("========================================\r\n");

    while (1) __asm__ volatile("wfi");
}
