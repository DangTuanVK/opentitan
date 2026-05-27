// SafeRoot HMAC-SHA256 + SHA-256 Functional Test
// Runs on Ibex CPU, drives HMAC HW accelerator via HAL
// Output: UART 115200 baud — PASS/FAIL for each test
//
// Test vectors:
//   SHA-256 empty string (NIST)
//   SHA-256 "abc" (NIST)
//   HMAC-SHA256 RFC 4231 Test Case 2

#include <stdint.h>
#include "../hal/hal.h"
#include "../hal/uart_drv.h"
#include "../hal/hmac_drv.h"

// ============================================================
// Helper: print uint32_t[8] digest as 64-char hex string
// HAL stores digest in big-endian word order, each word in
// native (little-endian) byte order. We print each word as
// 8 hex chars, MSB first, words in order [0]...[7].
// ============================================================
static void print_digest(const uint32_t digest[8]) {
    for (int i = 0; i < 8; i++) {
        uart_put_hex(digest[i]);
    }
}

// ============================================================
// Helper: compare two uint32_t[8] digests (constant-time)
// ============================================================
static int digest_equal(const uint32_t a[8], const uint32_t b[8]) {
    uint32_t diff = 0;
    for (int i = 0; i < 8; i++)
        diff |= a[i] ^ b[i];
    return diff == 0;
}

// ============================================================
// Helper: delay
// ============================================================
static void delay(uint32_t n) {
    volatile uint32_t i;
    for (i = 0; i < n; i++) __asm__ volatile("nop");
}

// ============================================================
// Test 1: SHA-256 of empty string
// Expected: e3b0c442 98fc1c14 9afbf4c8 996fb924
//           27ae41e4 649b934c a495991b 7852b855
// ============================================================
static const uint32_t sha256_empty_expected[8] = {
    0xe3b0c442, 0x98fc1c14, 0x9afbf4c8, 0x996fb924,
    0x27ae41e4, 0x649b934c, 0xa495991b, 0x7852b855
};

static int test_sha256_empty(void) {
    uint32_t digest[8];
    hal_status_t st;

    uart_puts("[TEST1] SHA-256 of empty string\r\n");
    uart_puts("  Expected: ");
    print_digest(sha256_empty_expected);
    uart_puts("\r\n");

    // SHA-256 with NULL data and zero length
    st = sha256((void *)0, 0, digest);
    if (st != kHalOk) {
        uart_puts("  FAIL: sha256() returned ");
        uart_put_hex((uint32_t)st);
        uart_puts("\r\n");
        return 0;
    }

    uart_puts("  Got:      ");
    print_digest(digest);
    uart_puts("\r\n");

    if (digest_equal(digest, sha256_empty_expected)) {
        uart_puts("  [PASS] SHA-256 empty string matches NIST vector\r\n");
        return 1;
    } else {
        uart_puts("  [FAIL] Digest mismatch!\r\n");
        return 0;
    }
}

// ============================================================
// Test 2: SHA-256 of "abc"
// Expected: ba7816bf 8f01cfea 414140de 5dae2223
//           b00361a3 96177a9c b410ff61 f20015ad
// ============================================================
static const uint32_t sha256_abc_expected[8] = {
    0xba7816bf, 0x8f01cfea, 0x414140de, 0x5dae2223,
    0xb00361a3, 0x96177a9c, 0xb410ff61, 0xf20015ad
};

// "abc\0" padded to 4 bytes for alignment
static const uint32_t abc_data[1] = { 0x00636261 };  // "abc" LE

static int test_sha256_abc(void) {
    uint32_t digest[8];
    hal_status_t st;

    uart_puts("[TEST2] SHA-256 of \"abc\"\r\n");
    uart_puts("  Expected: ");
    print_digest(sha256_abc_expected);
    uart_puts("\r\n");

    // SHA-256 of "abc" (3 bytes, but we pass 4-byte aligned word)
    // The HAL requires len as multiple of 4 for update, but sha256()
    // convenience function may handle padding internally.
    // We pass the 4-byte aligned buffer with len=4 (includes null byte).
    // For HW HMAC, the IP handles padding per SHA-256 spec.
    st = sha256((const void *)abc_data, 4, digest);
    if (st != kHalOk) {
        uart_puts("  FAIL: sha256() returned ");
        uart_put_hex((uint32_t)st);
        uart_puts("\r\n");
        return 0;
    }

    uart_puts("  Got:      ");
    print_digest(digest);
    uart_puts("\r\n");

    if (digest_equal(digest, sha256_abc_expected)) {
        uart_puts("  [PASS] SHA-256 \"abc\" matches NIST vector\r\n");
        return 1;
    } else {
        uart_puts("  [FAIL] Digest mismatch!\r\n");
        uart_puts("  Note: If mismatch, check byte ordering and padding\r\n");
        return 0;
    }
}

// ============================================================
// Test 3: HMAC-SHA256 — RFC 4231 Test Case 2
// Key:  "Jefe" (4 bytes, zero-padded to 32 bytes)
// Data: "what do ya want for nothing?" (28 bytes)
// Expected: 5bdcc146 bf60754e 6a042426 089575c7
//           5a003f08 9d273983 9dec58b9 64ec3843
// ============================================================
static const uint32_t hmac_rfc4231_expected[8] = {
    0x5bdcc146, 0xbf60754e, 0x6a042426, 0x089575c7,
    0x5a003f08, 0x9d273983, 0x9dec58b9, 0x64ec3843
};

// Key "Jefe" zero-padded to 256 bits (8 words)
// "Jefe" in LE = 0x6566654A
static const uint32_t hmac_key[8] = {
    0x6566654A, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};

// "what do ya want for nothing?" = 28 bytes
// Padded to 28 bytes (7 words, 28 is already a multiple of 4)
static const uint32_t hmac_data[7] = {
    0x74616877,  // "what" LE
    0x206f6420,  // " do " LE
    0x77206179,  // "ya w" LE
    0x20746e61,  // "ant " LE
    0x20726f66,  // "for " LE
    0x68746f6e,  // "noth" LE
    0x21676e69   // "ing!" LE — wait, it's "ing?" no: "nothing?" ends with ?
};

// Correct: "what do ya want for nothing?" = 28 chars
// w=77 h=68 a=61 t=74 => word0 = 0x74616877 (LE)
// ' '=20 d=64 o=6F ' '=20 => word1 = 0x206F6420
// y=79 a=61 ' '=20 w=77 => word2 = 0x77206179
// a=61 n=6E t=74 ' '=20 => word3 = 0x20746E61
// f=66 o=6F r=72 ' '=20 => word4 = 0x20726F66
// n=6E o=6F t=74 h=68 => word5 = 0x68746F6E
// i=69 n=6E g=67 ?=3F => word6 = 0x3F676E69
// Correction: last char is '?' not '!'
static const uint32_t hmac_msg[7] = {
    0x74616877,  // "what"
    0x206F6420,  // " do "
    0x77206179,  // "ya w"
    0x20746E61,  // "ant "
    0x20726F66,  // "for "
    0x68746F6E,  // "noth"
    0x3F676E69   // "ing?"
};

static int test_hmac_rfc4231(void) {
    uint32_t digest[8];
    hal_status_t st;

    uart_puts("[TEST3] HMAC-SHA256 RFC 4231 Test Case 2\r\n");
    uart_puts("  Key:      \"Jefe\"\r\n");
    uart_puts("  Data:     \"what do ya want for nothing?\"\r\n");
    uart_puts("  Expected: ");
    print_digest(hmac_rfc4231_expected);
    uart_puts("\r\n");

    // Initialize HMAC with key
    st = hmac_init(hmac_key);
    if (st != kHalOk) {
        uart_puts("  FAIL: hmac_init() returned ");
        uart_put_hex((uint32_t)st);
        uart_puts("\r\n");
        return 0;
    }

    // Feed message data (28 bytes = 7 words)
    st = hmac_update((const void *)hmac_msg, 28);
    if (st != kHalOk) {
        uart_puts("  FAIL: hmac_update() returned ");
        uart_put_hex((uint32_t)st);
        uart_puts("\r\n");
        return 0;
    }

    // Finalize and get digest
    st = hmac_final(digest);
    if (st != kHalOk) {
        uart_puts("  FAIL: hmac_final() returned ");
        uart_put_hex((uint32_t)st);
        uart_puts("\r\n");
        return 0;
    }

    uart_puts("  Got:      ");
    print_digest(digest);
    uart_puts("\r\n");

    if (digest_equal(digest, hmac_rfc4231_expected)) {
        uart_puts("  [PASS] HMAC-SHA256 matches RFC 4231 Test Case 2\r\n");
        return 1;
    } else {
        uart_puts("  [FAIL] HMAC digest mismatch!\r\n");
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
    uart_puts("  SafeRoot HMAC/SHA-256 Functional Test\r\n");
    uart_puts("  NIST + RFC 4231 vectors\r\n");
    uart_puts("========================================\r\n");
    uart_puts("\r\n");

    int pass = 0, fail = 0;

    if (test_sha256_empty()) pass++; else fail++;
    uart_puts("\r\n");

    if (test_sha256_abc()) pass++; else fail++;
    uart_puts("\r\n");

    if (test_hmac_rfc4231()) pass++; else fail++;

    uart_puts("\r\n");
    uart_puts("========================================\r\n");
    uart_puts("  Results: ");
    uart_put_hex((uint32_t)pass);
    uart_puts(" PASS, ");
    uart_put_hex((uint32_t)fail);
    uart_puts(" FAIL\r\n");

    if (fail == 0)
        uart_puts("  [ALL PASS] HMAC/SHA HW accelerator OK\r\n");
    else
        uart_puts("  [SOME FAIL] Check HMAC configuration\r\n");

    uart_puts("========================================\r\n");

    // Halt
    while (1) __asm__ volatile("wfi");
}
