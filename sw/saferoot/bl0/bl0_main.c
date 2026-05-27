// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// bl0_main.c — BL0 (ROM_EXT) Secure Boot entry point
//
// This is the first mutable boot stage in SafeRoot's secure boot chain.
// It is loaded from Flash at 0x20000000 by Boot ROM after hardware init.
//
// Boot flow:
//   1. Check lifecycle state (DEV / PROD / RMA only)
//   2. Read BL1 image header from Flash
//   3. Compute SHA-256 hash of BL1 payload
//   4. Verify ECDSA-P256 signature (stub)
//   5. Anti-rollback SVN check against OTP
//   6. DICE CDI_0 derivation: CDI_0 = HMAC-SHA256(UDS, BL1_hash || desc)
//   7. Advance key manager state machine
//   8. Jump to BL1 entry point

#include "bl0.h"
#include "dice.h"
#include "verify.h"
#include "../hal/hal.h"
#include "../hal/hmac_drv.h"
#include "../hal/otp_drv.h"
#include "../hal/flash_drv.h"
#include "../hal/uart_drv.h"
#include "../hal/gpio_drv.h"

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------
static bl0_error_t bl0_check_lifecycle(void);
static bl0_error_t bl0_read_bl1_header(fw_image_header_t *hdr);
static bl0_error_t bl0_verify_bl1(const fw_image_header_t *hdr);
static bl0_error_t bl0_derive_cdi(const uint32_t *bl1_hash, dice_cdi_t *cdi);
static bl0_error_t bl0_advance_keymgr(const dice_cdi_t *cdi);
static void        bl0_jump_to_bl1(const fw_image_header_t *hdr)
                       __attribute__((noreturn));
static void        bl0_fatal(bl0_error_t err) __attribute__((noreturn));

// ---------------------------------------------------------------------------
// BL0 main entry point — called by Boot ROM
// ---------------------------------------------------------------------------
void bl0_main(void)
{
    bl0_error_t err;
    fw_image_header_t bl1_hdr;
    dice_cdi_t cdi;

    uart_puts("\r\n========================================\r\n");
    uart_puts("SafeRoot BL0 (ROM_EXT) v1.0\r\n");
    uart_puts("========================================\r\n");

    // Indicate BL0 start via GPIO LED
    gpio_set_bit(GPIO_BOOT_STATUS_PIN);

    // -----------------------------------------------------------------------
    // Step 1: Check lifecycle state
    // -----------------------------------------------------------------------
    uart_puts("BL0 Step 1: Checking lifecycle state...\r\n");
    err = bl0_check_lifecycle();
    if (err != kBl0Ok) {
        bl0_fatal(err);
    }

    // -----------------------------------------------------------------------
    // Step 2: Read BL1 image header from Flash
    // -----------------------------------------------------------------------
    uart_puts("BL0 Step 2: Reading BL1 header from Flash...\r\n");
    err = bl0_read_bl1_header(&bl1_hdr);
    if (err != kBl0Ok) {
        bl0_fatal(err);
    }

    // -----------------------------------------------------------------------
    // Step 3 + 4 + 5: Verify BL1 image (hash, signature, anti-rollback)
    // -----------------------------------------------------------------------
    uart_puts("BL0 Step 3-5: Verifying BL1 image...\r\n");
    err = bl0_verify_bl1(&bl1_hdr);
    if (err != kBl0Ok) {
        bl0_fatal(err);
    }

    // -----------------------------------------------------------------------
    // Step 6: DICE CDI_0 derivation
    // -----------------------------------------------------------------------
    uart_puts("BL0 Step 6: Deriving DICE CDI_0...\r\n");
    err = bl0_derive_cdi(bl1_hdr.image_hash, &cdi);
    if (err != kBl0Ok) {
        bl0_fatal(err);
    }

    // -----------------------------------------------------------------------
    // Step 7: Advance Key Manager state
    // -----------------------------------------------------------------------
    uart_puts("BL0 Step 7: Advancing Key Manager...\r\n");
    err = bl0_advance_keymgr(&cdi);
    if (err != kBl0Ok) {
        bl0_fatal(err);
    }

    // Wipe CDI from memory after key manager has consumed it
    dice_wipe_secrets(&cdi);

    // -----------------------------------------------------------------------
    // Step 8: Jump to BL1
    // -----------------------------------------------------------------------
    uart_puts("BL0 Step 8: Jumping to BL1...\r\n");
    uart_puts("========================================\r\n\r\n");
    bl0_jump_to_bl1(&bl1_hdr);

    // Never reached
}

// ---------------------------------------------------------------------------
// Step 1: Read lifecycle state from LC_CTRL, allow DEV/PROD/RMA only
// ---------------------------------------------------------------------------
static bl0_error_t bl0_check_lifecycle(void)
{
    uint32_t lc_state = REG32_READ(LC_CTRL_BASE + LC_CTRL_STATE_REG);

    // Extract state field (bits [4:0] in typical encoding)
    uint32_t state_code = lc_state & 0x1FU;

    uart_puts("  LC state register: 0x");
    uart_put_hex(lc_state);
    uart_puts("\r\n");

    switch (state_code) {
    case LC_STATE_DEV:
        uart_puts("  Lifecycle: DEV\r\n");
        break;
    case LC_STATE_PROD:
        uart_puts("  Lifecycle: PROD\r\n");
        break;
    case LC_STATE_PROD_END:
        uart_puts("  Lifecycle: PROD_END\r\n");
        break;
    case LC_STATE_RMA:
        uart_puts("  Lifecycle: RMA\r\n");
        break;
    default:
        uart_puts("  Lifecycle: INVALID (0x");
        uart_put_hex(state_code);
        uart_puts(")\r\n");
        return kBl0ErrLcState;
    }

    return kBl0Ok;
}

// ---------------------------------------------------------------------------
// Step 2: Read BL1 image header from Flash and validate fields
// ---------------------------------------------------------------------------
static bl0_error_t bl0_read_bl1_header(fw_image_header_t *hdr)
{
    hal_status_t st;

    // Read header from flash (word-aligned read)
    uint32_t header_words = sizeof(fw_image_header_t) / 4U;
    st = flash_read(BL1_FLASH_BASE, (uint32_t *)hdr, header_words);
    if (st != kHalOk) {
        uart_puts("  Flash read failed\r\n");
        return kBl0ErrFlashRead;
    }

    // Validate magic number
    if (hdr->magic != FW_IMAGE_MAGIC) {
        uart_puts("  Bad magic: 0x");
        uart_put_hex(hdr->magic);
        uart_puts(" (expected 0x");
        uart_put_hex(FW_IMAGE_MAGIC);
        uart_puts(")\r\n");
        return kBl0ErrBadMagic;
    }

    // Validate image size
    if (hdr->image_size == 0 || hdr->image_size > BL1_MAX_SIZE) {
        uart_puts("  Bad image size: 0x");
        uart_put_hex(hdr->image_size);
        uart_puts("\r\n");
        return kBl0ErrBadSize;
    }

    // Ensure image_size is word-aligned
    if ((hdr->image_size & 0x3U) != 0) {
        uart_puts("  Image size not word-aligned\r\n");
        return kBl0ErrBadSize;
    }

    uart_puts("  BL1 header OK: v");
    uart_put_hex(hdr->version_major);
    uart_puts(".");
    uart_put_hex(hdr->version_minor);
    uart_puts(" size=0x");
    uart_put_hex(hdr->image_size);
    uart_puts(" svn=0x");
    uart_put_hex(hdr->security_version);
    uart_puts("\r\n");

    return kBl0Ok;
}

// ---------------------------------------------------------------------------
// Steps 3-5: Verify BL1 image (hash + signature + anti-rollback)
// ---------------------------------------------------------------------------
static bl0_error_t bl0_verify_bl1(const fw_image_header_t *hdr)
{
    // Image payload starts immediately after the header
    const uint8_t *payload = (const uint8_t *)(BL1_FLASH_BASE +
                              sizeof(fw_image_header_t));

    return verify_image(hdr, payload, hdr->image_size);
}

// ---------------------------------------------------------------------------
// Step 6: DICE CDI_0 derivation
// CDI_0 = HMAC-SHA256(UDS, BL1_hash || boot_descriptor)
// ---------------------------------------------------------------------------
static bl0_error_t bl0_derive_cdi(const uint32_t *bl1_hash, dice_cdi_t *cdi)
{
    // Read UDS (Unique Device Secret) from OTP Secret2 partition.
    // UDS is 256 bits = 8 words, read as 4 x 64-bit DAI reads.
    uint32_t uds[8];
    hal_status_t st;

    for (uint32_t i = 0; i < 4; i++) {
        st = otp_read_word64(OTP_UDS_ADDR + (i * 8),
                             &uds[i * 2], &uds[i * 2 + 1]);
        if (st != kHalOk) {
            uart_puts("  OTP read UDS failed at word ");
            uart_put_hex(i);
            uart_puts("\r\n");
            return kBl0ErrOtpRead;
        }
    }

    uart_puts("  UDS read from OTP OK\r\n");

    // Derive CDI_0
    bl0_error_t err = dice_derive_cdi((const uint8_t *)uds,
                                      (const uint8_t *)bl1_hash,
                                      cdi->cdi);
    // Wipe UDS from stack immediately
    for (uint32_t i = 0; i < 8; i++) {
        uds[i] = 0U;
    }
    HAL_BARRIER();

    if (err != kBl0Ok) {
        return err;
    }

    cdi->flags = 0U;  // No special flags for CDI_0
    return kBl0Ok;
}

// ---------------------------------------------------------------------------
// Step 7: Advance Key Manager state machine
// Write CDI into key manager SW binding registers, then trigger advance.
// ---------------------------------------------------------------------------
static bl0_error_t bl0_advance_keymgr(const dice_cdi_t *cdi)
{
    // Write CDI into Key Manager SW binding registers (8 x 32-bit)
    const uint32_t *cdi_words = (const uint32_t *)cdi->cdi;
    for (uint32_t i = 0; i < SHA256_DIGEST_WORDS; i++) {
        REG32_WRITE(KEYMGR_BASE + KEYMGR_SW_BINDING_0 + (i * 4),
                    cdi_words[i]);
    }

    // Trigger key manager state advance (write 1 to START)
    REG32_WRITE(KEYMGR_BASE + KEYMGR_START_REG, 0x1U);

    // Poll for completion (check working state != busy)
    uint32_t timeout = HAL_TIMEOUT_CYCLES;
    while (timeout > 0) {
        uint32_t state = REG32_READ(KEYMGR_BASE + KEYMGR_WORKING_STATE);
        if ((state & 0x1U) == 0) {
            // Not busy
            uart_puts("  Key Manager advanced OK\r\n");
            return kBl0Ok;
        }
        timeout--;
    }

    uart_puts("  Key Manager advance TIMEOUT\r\n");
    return kBl0ErrKeymgr;
}

// ---------------------------------------------------------------------------
// Step 8: Jump to BL1 entry point
// ---------------------------------------------------------------------------
static void bl0_jump_to_bl1(const fw_image_header_t *hdr)
{
    // BL1 entry point = BL1 base + header size + entry offset
    uint32_t entry_addr = BL1_FLASH_BASE + sizeof(fw_image_header_t) +
                          hdr->entry_offset;

    uart_puts("  Jumping to 0x");
    uart_put_hex(entry_addr);
    uart_puts("\r\n");

    // Memory barrier before jump
    HAL_BARRIER();

    // Cast to function pointer and call (noreturn)
    typedef void (*entry_fn_t)(void);
    entry_fn_t entry = (entry_fn_t)entry_addr;
    entry();

    // Should never reach here — infinite loop as safety net
    while (1) {
        __asm__ volatile("wfi");
    }
}

// ---------------------------------------------------------------------------
// Fatal error handler: print error code and halt
// ---------------------------------------------------------------------------
static void bl0_fatal(bl0_error_t err)
{
    uart_puts("\r\n!!! BL0 FATAL ERROR: 0x");
    uart_put_hex((uint32_t)err);
    uart_puts(" !!!\r\n");

    // Blink GPIO to indicate error (toggle pattern)
    gpio_clear_bit(GPIO_BOOT_STATUS_PIN);

    // Halt — do not proceed to BL1
    while (1) {
        __asm__ volatile("wfi");
    }
}
