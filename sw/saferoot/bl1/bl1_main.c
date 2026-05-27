// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// bl1_main.c — BL1 (Owner Firmware) Secure Boot entry point
//
// This is the second mutable boot stage in SafeRoot's secure boot chain.
// It is called by BL0 after DICE CDI_0 derivation, running from Flash
// at 0x20008000.
//
// Boot flow:
//   1. Print BL1 banner via UART
//   2. Read Application image header from Flash
//   3. Verify App image (hash + signature using Owner key)
//   4. Anti-rollback check (BL1 SVN from OTP)
//   5. Derive CDI_1 = HMAC-SHA256(CDI_0, App_hash || "SafeRoot.BL1.CDI_1")
//   6. A/B bank selection (pick valid + newer firmware)
//   7. Initialize Watchdog timer
//   8. Jump to Application entry point, passing CDI_1

#include "bl1.h"
#include "bl1_bank.h"
#include "../bl0/dice.h"
#include "../bl0/verify.h"
#include "../hal/hal.h"
#include "../hal/hmac_drv.h"
#include "../hal/otp_drv.h"
#include "../hal/flash_drv.h"
#include "../hal/uart_drv.h"
#include "../hal/gpio_drv.h"
#include "../wdt/wdt.h"

// ---------------------------------------------------------------------------
// BL1 CDI descriptor (padded to 4-byte alignment for HMAC FIFO)
// ---------------------------------------------------------------------------
static const uint8_t bl1_cdi_descriptor[20] __attribute__((aligned(4))) = {
    // "SafeRoot.BL1.CDI_1" = 18 bytes + 2 bytes zero padding
    'S','a','f','e','R','o','o','t','.','B','L','1','.','C','D','I','_','1',
    0x00, 0x00
};

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------
static bl1_error_t bl1_read_app_header(uint32_t base, fw_image_header_t *hdr);
static bl1_error_t bl1_verify_app(uint32_t base, const fw_image_header_t *hdr);
static bl1_error_t bl1_check_antirollback(uint32_t image_svn);
static bl1_error_t bl1_derive_cdi1(const dice_cdi_t *cdi_0,
                                    const uint32_t *app_hash,
                                    dice_cdi_t *cdi_1);
static void        bl1_jump_to_app(uint32_t base,
                                    const fw_image_header_t *hdr,
                                    const dice_cdi_t *cdi_1)
                       __attribute__((noreturn));
static void        bl1_fatal(bl1_error_t err) __attribute__((noreturn));

// ---------------------------------------------------------------------------
// BL1 main entry point — called by BL0
// ---------------------------------------------------------------------------
void bl1_main(const dice_cdi_t *cdi_0)
{
    bl1_error_t err;
    fw_image_header_t app_hdr;
    dice_cdi_t cdi_1;
    bank_info_t active_bank;
    uint32_t retries = 0;

    // -------------------------------------------------------------------
    // Step 1: Print BL1 banner
    // -------------------------------------------------------------------
    uart_puts("\r\n========================================\r\n");
    uart_puts("SafeRoot BL1 (Owner FW) v1.0\r\n");
    uart_puts("========================================\r\n");

    gpio_set_bit(GPIO_BOOT_STATUS_PIN);

retry_boot:
    // -------------------------------------------------------------------
    // Step 6 (early): A/B bank selection — determines which bank to verify
    // -------------------------------------------------------------------
    uart_puts("BL1 Step 1: Selecting firmware bank...\r\n");
    err = bank_select(&active_bank);
    if (err != kBl1Ok) {
        goto boot_fail;
    }

    uint32_t app_base = active_bank.base_addr;

    // -------------------------------------------------------------------
    // Step 2: Read Application image header from Flash
    // -------------------------------------------------------------------
    uart_puts("BL1 Step 2: Reading App header from Flash...\r\n");
    err = bl1_read_app_header(app_base, &app_hdr);
    if (err != kBl1Ok) {
        goto boot_fail;
    }

    // -------------------------------------------------------------------
    // Step 3: Verify App image (hash + signature)
    // -------------------------------------------------------------------
    uart_puts("BL1 Step 3: Verifying App image...\r\n");
    err = bl1_verify_app(app_base, &app_hdr);
    if (err != kBl1Ok) {
        goto boot_fail;
    }

    // -------------------------------------------------------------------
    // Step 4: Anti-rollback check
    // -------------------------------------------------------------------
    uart_puts("BL1 Step 4: Anti-rollback check...\r\n");
    err = bl1_check_antirollback(app_hdr.security_version);
    if (err != kBl1Ok) {
        goto boot_fail;
    }

    // -------------------------------------------------------------------
    // Step 5: Derive CDI_1 = HMAC-SHA256(CDI_0, App_hash || descriptor)
    // -------------------------------------------------------------------
    uart_puts("BL1 Step 5: Deriving DICE CDI_1...\r\n");
    err = bl1_derive_cdi1(cdi_0, app_hdr.image_hash, &cdi_1);
    if (err != kBl1Ok) {
        goto boot_fail;
    }

    // Mark this bank as verified-good
    bank_mark_good(active_bank.id);

    // -------------------------------------------------------------------
    // Step 7: Initialize Watchdog
    // -------------------------------------------------------------------
    uart_puts("BL1 Step 7: Initializing Watchdog...\r\n");
    wdt_init(BL1_WDT_BARK_TIMEOUT, BL1_WDT_BITE_TIMEOUT);
    wdt_enable();
    uart_puts("  WDT enabled (bark=5s, bite=8s)\r\n");

    // -------------------------------------------------------------------
    // Step 8: Jump to Application entry point
    // -------------------------------------------------------------------
    uart_puts("BL1 Step 8: Jumping to Application...\r\n");
    uart_puts("========================================\r\n\r\n");
    bl1_jump_to_app(app_base, &app_hdr, &cdi_1);

    // Never reached

boot_fail:
    retries++;
    uart_puts("BL1: Boot attempt failed (retry ");
    uart_put_hex(retries);
    uart_puts("/");
    uart_put_hex(BL1_MAX_RETRIES);
    uart_puts(")\r\n");

    if (retries < BL1_MAX_RETRIES) {
        goto retry_boot;
    }

    uart_puts("BL1: All retries exhausted\r\n");
    bl1_fatal(kBl1ErrRetryExhaust);
}

// ---------------------------------------------------------------------------
// Step 2: Read App image header and validate fields
// ---------------------------------------------------------------------------
static bl1_error_t bl1_read_app_header(uint32_t base, fw_image_header_t *hdr)
{
    hal_status_t st;

    uint32_t header_words = sizeof(fw_image_header_t) / 4U;
    st = flash_read(base, (uint32_t *)hdr, header_words);
    if (st != kHalOk) {
        uart_puts("  Flash read failed\r\n");
        return kBl1ErrFlashRead;
    }

    if (hdr->magic != FW_IMAGE_MAGIC) {
        uart_puts("  Bad magic: 0x");
        uart_put_hex(hdr->magic);
        uart_puts("\r\n");
        return kBl1ErrBadMagic;
    }

    if (hdr->image_size == 0 || hdr->image_size > APP_MAX_SIZE) {
        uart_puts("  Bad image size: 0x");
        uart_put_hex(hdr->image_size);
        uart_puts("\r\n");
        return kBl1ErrBadSize;
    }

    if ((hdr->image_size & 0x3U) != 0) {
        uart_puts("  Image size not word-aligned\r\n");
        return kBl1ErrBadSize;
    }

    uart_puts("  App header OK: v");
    uart_put_hex(hdr->version_major);
    uart_puts(".");
    uart_put_hex(hdr->version_minor);
    uart_puts(" size=0x");
    uart_put_hex(hdr->image_size);
    uart_puts(" svn=0x");
    uart_put_hex(hdr->security_version);
    uart_puts("\r\n");

    return kBl1Ok;
}

// ---------------------------------------------------------------------------
// Step 3: Verify App image using BL0's verify_image (hash + sig)
// ---------------------------------------------------------------------------
static bl1_error_t bl1_verify_app(uint32_t base, const fw_image_header_t *hdr)
{
    const uint8_t *payload = (const uint8_t *)(base +
                              sizeof(fw_image_header_t));

    bl0_error_t v_err = verify_image(hdr, payload, hdr->image_size);

    // Map BL0 error codes to BL1 error codes
    switch (v_err) {
    case kBl0Ok:
        return kBl1Ok;
    case kBl0ErrHashMismatch:
        return kBl1ErrHashMismatch;
    case kBl0ErrSigVerify:
        return kBl1ErrSigVerify;
    case kBl0ErrAntiRollback:
        return kBl1ErrAntiRollback;
    default:
        return kBl1ErrHashMismatch;
    }
}

// ---------------------------------------------------------------------------
// Step 4: Anti-rollback check against BL1-level OTP SVN
// ---------------------------------------------------------------------------
static bl1_error_t bl1_check_antirollback(uint32_t image_svn)
{
    uint32_t min_svn = 0;
    hal_status_t st;

    st = otp_read_word(OTP_BL1_SVN_ADDR, &min_svn);
    if (st != kHalOk) {
        uart_puts("  OTP read BL1 min_svn failed\r\n");
        return kBl1ErrOtpRead;
    }

    uart_puts("  OTP min_svn=0x");
    uart_put_hex(min_svn);
    uart_puts(" image_svn=0x");
    uart_put_hex(image_svn);
    uart_puts("\r\n");

    if (image_svn < min_svn) {
        uart_puts("  Anti-rollback FAIL\r\n");
        return kBl1ErrAntiRollback;
    }

    uart_puts("  Anti-rollback OK\r\n");
    return kBl1Ok;
}

// ---------------------------------------------------------------------------
// Step 5: Derive CDI_1 = HMAC-SHA256(CDI_0, App_hash || descriptor)
// ---------------------------------------------------------------------------
static bl1_error_t bl1_derive_cdi1(const dice_cdi_t *cdi_0,
                                    const uint32_t *app_hash,
                                    dice_cdi_t *cdi_1)
{
    hal_status_t st;

    // Use CDI_0 as HMAC key
    st = hmac_init((const uint32_t *)cdi_0->cdi);
    if (st != kHalOk) {
        uart_puts("  DICE hmac_init failed\r\n");
        return kBl1ErrDice;
    }

    // Feed App hash (32 bytes)
    st = hmac_update((const uint8_t *)app_hash, SHA256_DIGEST_BYTES);
    if (st != kHalOk) {
        uart_puts("  DICE hmac_update(hash) failed\r\n");
        hmac_wipe(0x0);
        return kBl1ErrDice;
    }

    // Feed BL1 CDI descriptor (padded to 20 bytes for 4-byte alignment)
    st = hmac_update(bl1_cdi_descriptor, 20U);
    if (st != kHalOk) {
        uart_puts("  DICE hmac_update(desc) failed\r\n");
        hmac_wipe(0x0);
        return kBl1ErrDice;
    }

    // Finalize: read 256-bit digest = CDI_1
    uint32_t digest[SHA256_DIGEST_WORDS];
    st = hmac_final(digest);
    if (st != kHalOk) {
        uart_puts("  DICE hmac_final failed\r\n");
        hmac_wipe(0x0);
        return kBl1ErrDice;
    }

    // Copy digest to CDI_1
    const uint8_t *src = (const uint8_t *)digest;
    for (uint32_t i = 0; i < DICE_CDI_BYTES; i++) {
        cdi_1->cdi[i] = src[i];
    }
    cdi_1->flags = 0U;

    // Wipe digest from stack
    for (uint32_t i = 0; i < SHA256_DIGEST_WORDS; i++) {
        digest[i] = 0U;
    }
    HAL_BARRIER();

    // Wipe HMAC engine internal state
    hmac_wipe(0xA5A5A5A5);

    uart_puts("  DICE CDI_1 derived OK\r\n");
    return kBl1Ok;
}

// ---------------------------------------------------------------------------
// Step 8: Jump to Application entry point, passing CDI_1
// ---------------------------------------------------------------------------
static void bl1_jump_to_app(uint32_t base,
                             const fw_image_header_t *hdr,
                             const dice_cdi_t *cdi_1)
{
    // App entry point = bank base + header size + entry offset
    uint32_t entry_addr = base + sizeof(fw_image_header_t) +
                          hdr->entry_offset;

    uart_puts("  Jumping to 0x");
    uart_put_hex(entry_addr);
    uart_puts("\r\n");

    // Memory barrier before jump
    HAL_BARRIER();

    // Cast to function pointer: Application receives CDI_1
    typedef void (*app_entry_fn_t)(const dice_cdi_t *cdi);
    app_entry_fn_t entry = (app_entry_fn_t)entry_addr;
    entry(cdi_1);

    // Should never reach here — infinite loop as safety net
    while (1) {
        __asm__ volatile("wfi");
    }
}

// ---------------------------------------------------------------------------
// Fatal error handler: print error code and halt
// ---------------------------------------------------------------------------
static void bl1_fatal(bl1_error_t err)
{
    uart_puts("\r\n!!! BL1 FATAL ERROR: 0x");
    uart_put_hex((uint32_t)err);
    uart_puts(" !!!\r\n");

    gpio_clear_bit(GPIO_BOOT_STATUS_PIN);

    // Halt — do not proceed to Application
    while (1) {
        __asm__ volatile("wfi");
    }
}
