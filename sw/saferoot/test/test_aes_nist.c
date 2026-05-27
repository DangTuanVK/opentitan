// SafeRoot AES Functional Test — NIST FIPS 197 Appendix C.3
// Runs on Ibex CPU, drives AES HW accelerator, verifies encrypt/decrypt
// Output: UART 115200 baud — PASS/FAIL for each test

#include <stdint.h>
#include "../hal/hal.h"
#include "../hal/uart_drv.h"
#include "../hal/aes_drv.h"

// ============================================================
// NIST FIPS 197 Appendix C.3 — AES-256
// Key:        000102030405060708090a0b0c0d0e0f
//             101112131415161718191a1b1c1d1e1f
// Plaintext:  00112233445566778899aabbccddeeff
// Ciphertext: 8ea2b7ca516745bfeafc49904b496089
// ============================================================

static const uint8_t nist_key[32] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

static const uint8_t nist_plaintext[16] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

static const uint8_t nist_ciphertext[16] = {
    0x8e, 0xa2, 0xb7, 0xca, 0x51, 0x67, 0x45, 0xbf,
    0xea, 0xfc, 0x49, 0x90, 0x4b, 0x49, 0x60, 0x89
};

// ============================================================
// Helper: print hex byte array
// ============================================================
static void print_hex(const uint8_t *data, int len) {
    const char hex[] = "0123456789abcdef";
    for (int i = 0; i < len; i++) {
        uart_putc(hex[data[i] >> 4]);
        uart_putc(hex[data[i] & 0xF]);
    }
}

// ============================================================
// Helper: compare byte arrays (constant-time)
// ============================================================
static int mem_equal(const uint8_t *a, const uint8_t *b, int len) {
    uint8_t diff = 0;
    for (int i = 0; i < len; i++)
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
// Test 1: AES-256 Encrypt (NIST vector)
// ============================================================
static int test_aes_encrypt(void) {
    uint8_t result[16];
    hal_status_t st;

    uart_puts("[TEST1] AES-256 Encrypt (NIST FIPS 197 C.3)\r\n");
    uart_puts("  Key:       ");
    print_hex(nist_key, 32);
    uart_puts("\r\n");
    uart_puts("  Plaintext: ");
    print_hex(nist_plaintext, 16);
    uart_puts("\r\n");
    uart_puts("  Expected:  ");
    print_hex(nist_ciphertext, 16);
    uart_puts("\r\n");

    // Initialize AES: encrypt, ECB mode, 256-bit key
    st = aes_init(kAesEncrypt, kAesModeEcb, nist_key, (void*)0);
    if (st != kHalOk) {
        uart_puts("  FAIL: aes_init returned ");
        uart_put_hex(st);
        uart_puts("\r\n");
        return 0;
    }

    // Process one block
    st = aes_process_block(nist_plaintext, result);
    if (st != kHalOk) {
        uart_puts("  FAIL: aes_process_block returned ");
        uart_put_hex(st);
        uart_puts("\r\n");
        aes_clear();
        return 0;
    }

    aes_clear();

    uart_puts("  Got:       ");
    print_hex(result, 16);
    uart_puts("\r\n");

    if (mem_equal(result, nist_ciphertext, 16)) {
        uart_puts("  [PASS] AES-256 Encrypt matches NIST vector\r\n");
        return 1;
    } else {
        uart_puts("  [FAIL] Ciphertext mismatch!\r\n");
        return 0;
    }
}

// ============================================================
// Test 2: AES-256 Decrypt (verify roundtrip)
// ============================================================
static int test_aes_decrypt(void) {
    uint8_t result[16];
    hal_status_t st;

    uart_puts("[TEST2] AES-256 Decrypt (roundtrip)\r\n");

    st = aes_init(kAesDecrypt, kAesModeEcb, nist_key, (void*)0);
    if (st != kHalOk) {
        uart_puts("  FAIL: aes_init returned ");
        uart_put_hex(st);
        uart_puts("\r\n");
        return 0;
    }

    st = aes_process_block(nist_ciphertext, result);
    if (st != kHalOk) {
        uart_puts("  FAIL: aes_process_block returned ");
        uart_put_hex(st);
        uart_puts("\r\n");
        aes_clear();
        return 0;
    }

    aes_clear();

    uart_puts("  Decrypted: ");
    print_hex(result, 16);
    uart_puts("\r\n");

    if (mem_equal(result, nist_plaintext, 16)) {
        uart_puts("  [PASS] Decrypt matches original plaintext\r\n");
        return 1;
    } else {
        uart_puts("  [FAIL] Plaintext mismatch!\r\n");
        return 0;
    }
}

// ============================================================
// Test 3: AES encrypt then decrypt (full roundtrip)
// ============================================================
static int test_aes_roundtrip(void) {
    uint8_t encrypted[16], decrypted[16];
    hal_status_t st;

    // Custom test data
    uint8_t test_data[16] = {
        0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF
    };

    uart_puts("[TEST3] AES-256 Full roundtrip (encrypt then decrypt)\r\n");
    uart_puts("  Input:     ");
    print_hex(test_data, 16);
    uart_puts("\r\n");

    // Encrypt
    st = aes_init(kAesEncrypt, kAesModeEcb, nist_key, (void*)0);
    if (st != kHalOk) { uart_puts("  FAIL: encrypt init\r\n"); return 0; }
    st = aes_process_block(test_data, encrypted);
    if (st != kHalOk) { uart_puts("  FAIL: encrypt\r\n"); aes_clear(); return 0; }
    aes_clear();

    uart_puts("  Encrypted: ");
    print_hex(encrypted, 16);
    uart_puts("\r\n");

    // Decrypt
    st = aes_init(kAesDecrypt, kAesModeEcb, nist_key, (void*)0);
    if (st != kHalOk) { uart_puts("  FAIL: decrypt init\r\n"); return 0; }
    st = aes_process_block(encrypted, decrypted);
    if (st != kHalOk) { uart_puts("  FAIL: decrypt\r\n"); aes_clear(); return 0; }
    aes_clear();

    uart_puts("  Decrypted: ");
    print_hex(decrypted, 16);
    uart_puts("\r\n");

    if (mem_equal(decrypted, test_data, 16)) {
        uart_puts("  [PASS] Roundtrip matches\r\n");
        return 1;
    } else {
        uart_puts("  [FAIL] Roundtrip mismatch!\r\n");
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
    uart_puts("  SafeRoot AES Functional Test\r\n");
    uart_puts("  NIST FIPS 197 + roundtrip\r\n");
    uart_puts("========================================\r\n");
    uart_puts("\r\n");

    int pass = 0, fail = 0;

    if (test_aes_encrypt()) pass++; else fail++;
    uart_puts("\r\n");

    if (test_aes_decrypt()) pass++; else fail++;
    uart_puts("\r\n");

    if (test_aes_roundtrip()) pass++; else fail++;

    uart_puts("\r\n");
    uart_puts("========================================\r\n");
    uart_puts("  Results: ");
    uart_put_hex(pass);
    uart_puts(" PASS, ");
    uart_put_hex(fail);
    uart_puts(" FAIL\r\n");

    if (fail == 0)
        uart_puts("  [ALL PASS] AES HW accelerator OK\r\n");
    else
        uart_puts("  [SOME FAIL] Check AES configuration\r\n");

    uart_puts("========================================\r\n");

    // Halt
    while (1) __asm__ volatile("wfi");
}
