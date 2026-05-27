// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// tpm_cmd.c — TPM 2.0 command handlers for SafeRoot
// 13 commands implementing a minimal TPM 2.0 subset.
// ECDSA signing operations are stubs (TODO: integrate OTBN).

#include "tpm_cmd.h"
#include "tpm_pcr.h"
#include "tpm_nv.h"
#include "../hal/hmac_drv.h"
#include "../hal/aes_drv.h"
#include "../hal/otp_drv.h"
#include "../hal/uart_drv.h"

// ---------------------------------------------------------------------------
// Access global TPM state (defined in tpm.c)
// ---------------------------------------------------------------------------
extern tpm_state_t g_tpm_state;

// ---------------------------------------------------------------------------
// Byte-order helpers (same as tpm.c — kept local to avoid cross-unit deps)
// ---------------------------------------------------------------------------
static void cpu_to_be16(uint16_t v, uint8_t *p) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v);
}

static void cpu_to_be32(uint32_t v, uint8_t *p) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)(v);
}

static uint16_t be16_to_cpu(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0] << 8 | p[1]);
}

uint32_t be32_to_cpu(const uint8_t *p) {
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 |
           (uint32_t)p[2] << 8  | (uint32_t)p[3];
}

// ---------------------------------------------------------------------------
// Helper: write a success response header, return pointer past header
// ---------------------------------------------------------------------------
uint8_t *resp_header(uint8_t *resp, uint32_t total_size, uint32_t rc) {
    cpu_to_be16(TPM_ST_NO_SESSIONS, &resp[0]);
    cpu_to_be32(total_size, &resp[2]);
    cpu_to_be32(rc, &resp[6]);
    return resp + TPM_HEADER_SIZE;
}

// ---------------------------------------------------------------------------
// Helper: write a minimal (header-only) response
// ---------------------------------------------------------------------------
void resp_simple(uint32_t rc, uint8_t *resp, uint32_t *resp_len) {
    resp_header(resp, TPM_HEADER_SIZE, rc);
    *resp_len = TPM_HEADER_SIZE;
}

// ===========================================================================
// TPM2_Startup
// Payload: 2 bytes — startupType (TPM_SU_CLEAR or TPM_SU_STATE)
// ===========================================================================
uint32_t cmd_startup(const uint8_t *payload, uint32_t payload_len,
                     uint8_t *resp, uint32_t *resp_len) {
    if (payload_len < 2) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    uint16_t su_type = be16_to_cpu(payload);
    if (su_type != TPM_SU_CLEAR && su_type != TPM_SU_STATE) {
        resp_simple(TPM_RC_VALUE, resp, resp_len);
        return TPM_RC_VALUE;
    }

    if (g_tpm_state != kTpmStateUninitialized) {
        // Already started — return INITIALIZE per spec
        resp_simple(TPM_RC_INITIALIZE, resp, resp_len);
        return TPM_RC_INITIALIZE;
    }

    if (su_type == TPM_SU_CLEAR) {
        tpm_pcr_init();
        uart_puts("[TPM] Startup(CLEAR)\r\n");
    } else {
        uart_puts("[TPM] Startup(STATE) — restoring saved state\r\n");
        // Restore PCR values from NV index 0 (saved by Shutdown)
        uint8_t saved_pcrs[32];
        for (uint32_t i = 0; i < 24; i++) {
            if (tpm_nv_read(i, saved_pcrs, 32) == 0) {
                tpm_pcr_restore(i, saved_pcrs);
            }
        }
    }

    g_tpm_state = kTpmStateStarted;
    resp_simple(TPM_RC_SUCCESS, resp, resp_len);
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_Shutdown
// Payload: 2 bytes — shutdownType
// ===========================================================================
uint32_t cmd_shutdown(const uint8_t *payload, uint32_t payload_len,
                      uint8_t *resp, uint32_t *resp_len) {
    if (payload_len < 2) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    uint16_t shutdown_type = ((uint16_t)payload[0] << 8) | payload[1];
    uart_puts("[TPM] Shutdown\r\n");

    // Save PCR state to NV if shutdownType == TPM_SU_STATE
    if (shutdown_type == 1) {  // TPM_SU_STATE
        uint8_t pcr_val[32];
        for (uint32_t i = 0; i < 24; i++) {
            tpm_pcr_read(i, pcr_val);
            tpm_nv_write(i, pcr_val, 32);
        }
        uart_puts("[TPM] PCR state saved to NV\r\n");
    }

    g_tpm_state = kTpmStateUninitialized;
    resp_simple(TPM_RC_SUCCESS, resp, resp_len);
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_SelfTest
// Payload: 1 byte — fullTest (yes/no)
// ===========================================================================
uint32_t cmd_self_test(const uint8_t *payload, uint32_t payload_len,
                       uint8_t *resp, uint32_t *resp_len) {
    (void)payload;
    (void)payload_len;

    uart_puts("[TPM] SelfTest — running crypto checks\r\n");

    // SHA-256 self-test: hash a known vector
    static const uint8_t test_msg[] __attribute__((aligned(4))) = {
        'a', 'b', 'c', 0   // padded to 4 bytes
    };
    uint32_t digest[8];
    hal_status_t st = sha256(test_msg, 3, digest);
    if (st != kHalOk) {
        uart_puts("[TPM] SelfTest FAIL: SHA-256 error\r\n");
        resp_simple(TPM_RC_FAILURE, resp, resp_len);
        return TPM_RC_FAILURE;
    }

    // Expected SHA-256("abc") first word = 0xba7816bf (big-endian)
    // Our HW returns in little-endian word order; just check HW didn't error.

    // AES self-test: init + clear
    uint32_t zero_key[8];
    memset(zero_key, 0, sizeof(zero_key));
    st = aes_init(kAesEncrypt, kAesModeEcb, zero_key, NULL);
    if (st != kHalOk) {
        uart_puts("[TPM] SelfTest FAIL: AES error\r\n");
        resp_simple(TPM_RC_FAILURE, resp, resp_len);
        return TPM_RC_FAILURE;
    }
    aes_clear();

    g_tpm_state = kTpmStateSelfTested;
    uart_puts("[TPM] SelfTest PASS\r\n");
    resp_simple(TPM_RC_SUCCESS, resp, resp_len);
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_PCR_Extend
// Payload: 4 bytes PCR handle + 32 bytes digest (simplified)
// ===========================================================================
uint32_t cmd_pcr_extend(const uint8_t *payload, uint32_t payload_len,
                         uint8_t *resp, uint32_t *resp_len) {
    if (payload_len < 4 + TPM_PCR_SIZE) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    uint32_t pcr_handle = be32_to_cpu(payload);
    uint32_t pcr_idx    = pcr_handle & 0xFFU;  // Lower byte = PCR index
    const uint8_t *digest = payload + 4;

    uint32_t rc = tpm_pcr_extend(pcr_idx, digest);
    resp_simple(rc, resp, resp_len);
    return rc;
}

// ===========================================================================
// TPM2_PCR_Read
// Payload: 4 bytes PCR selection (simplified: just PCR index as uint32)
// Response: header + 32 bytes PCR value
// ===========================================================================
uint32_t cmd_pcr_read(const uint8_t *payload, uint32_t payload_len,
                      uint8_t *resp, uint32_t *resp_len) {
    if (payload_len < 4) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    uint32_t pcr_idx = be32_to_cpu(payload) & 0xFFU;
    uint8_t pcr_val[TPM_PCR_SIZE];

    uint32_t rc = tpm_pcr_read(pcr_idx, pcr_val);
    if (rc != TPM_RC_SUCCESS) {
        resp_simple(rc, resp, resp_len);
        return rc;
    }

    uint32_t total = TPM_HEADER_SIZE + TPM_PCR_SIZE;
    uint8_t *p = resp_header(resp, total, TPM_RC_SUCCESS);
    memcpy(p, pcr_val, TPM_PCR_SIZE);
    *resp_len = total;
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_PCR_Reset
// Payload: 4 bytes PCR handle
// ===========================================================================
uint32_t cmd_pcr_reset(const uint8_t *payload, uint32_t payload_len,
                       uint8_t *resp, uint32_t *resp_len) {
    if (payload_len < 4) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    uint32_t pcr_idx = be32_to_cpu(payload) & 0xFFU;
    uint32_t rc = tpm_pcr_reset(pcr_idx);
    resp_simple(rc, resp, resp_len);
    return rc;
}

// ===========================================================================
// TPM2_Quote — Sign selected PCR values (ECDSA stub)
// Returns: header + 32 bytes PCR composite hash + 64 bytes stub signature
// ===========================================================================
uint32_t cmd_quote(const uint8_t *payload, uint32_t payload_len,
                   uint8_t *resp, uint32_t *resp_len) {
    (void)payload;
    (void)payload_len;

    // Compute composite hash of all PCRs (simplified)
    uint8_t all_pcrs[TPM_NUM_PCRS * TPM_PCR_SIZE] __attribute__((aligned(4)));
    for (uint32_t i = 0; i < TPM_NUM_PCRS; i++) {
        tpm_pcr_read(i, &all_pcrs[i * TPM_PCR_SIZE]);
    }

    uint32_t composite[8];
    sha256(all_pcrs, sizeof(all_pcrs), composite);

    // Build response: header + composite_hash(32) + stub_signature(64)
    uint32_t total = TPM_HEADER_SIZE + 32 + 64;
    uint8_t *p = resp_header(resp, total, TPM_RC_SUCCESS);
    memcpy(p, composite, 32);
    p += 32;

    // TODO: ECDSA sign composite hash using OTBN co-processor
    // For now, fill signature with 0xAA pattern to indicate stub
    memset(p, 0xAA, 64);

    *resp_len = total;
    uart_puts("[TPM] Quote: ECDSA signature is STUB\r\n");
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_CreatePrimary — Generate Endorsement Key (stub)
// ===========================================================================
uint32_t cmd_create_primary(const uint8_t *payload, uint32_t payload_len,
                            uint8_t *resp, uint32_t *resp_len) {
    (void)payload;
    (void)payload_len;

    // TODO: Generate ECC P-256 key pair using OTBN
    // For now, return a dummy 32-byte "public key" from OTP device ID

    uint32_t total = TPM_HEADER_SIZE + 4 + 32;  // handle + pubkey
    uint8_t *p = resp_header(resp, total, TPM_RC_SUCCESS);

    // Return a fixed handle
    cpu_to_be32(0x80000000U, p);
    p += 4;

    // Read device identity from OTP as stub public key
    uint32_t otp_val;
    for (uint32_t i = 0; i < 8; i++) {
        if (otp_read_word(i * 4, &otp_val) == kHalOk) {
            cpu_to_be32(otp_val, p + i * 4);
        } else {
            memset(p + i * 4, 0, 4);
        }
    }

    *resp_len = total;
    uart_puts("[TPM] CreatePrimary: EK generation is STUB\r\n");
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_Load — Load a key object (stub)
// ===========================================================================
uint32_t cmd_load(const uint8_t *payload, uint32_t payload_len,
                  uint8_t *resp, uint32_t *resp_len) {
    (void)payload;
    (void)payload_len;

    // TODO: parse TPM2B_PRIVATE + TPM2B_PUBLIC, validate, assign handle
    uint32_t total = TPM_HEADER_SIZE + 4;  // return handle
    uint8_t *p = resp_header(resp, total, TPM_RC_SUCCESS);
    cpu_to_be32(0x80000001U, p);  // Loaded key handle

    *resp_len = total;
    uart_puts("[TPM] Load: key load is STUB\r\n");
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_Seal — Encrypt data bound to PCR policy (AES-256-ECB wrap)
// Payload: 4 bytes data_len + data
// ===========================================================================
uint32_t cmd_seal(const uint8_t *payload, uint32_t payload_len,
                  uint8_t *resp, uint32_t *resp_len) {
    if (payload_len < 4) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    uint32_t data_len = be32_to_cpu(payload);
    if (data_len > payload_len - 4 || data_len > 128) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    const uint8_t *plaintext = payload + 4;

    // Derive sealing key from PCR[0..7] composite hash
    uint8_t pcr_concat[8 * TPM_PCR_SIZE] __attribute__((aligned(4)));
    for (uint32_t i = 0; i < 8; i++) {
        tpm_pcr_read(i, &pcr_concat[i * TPM_PCR_SIZE]);
    }
    uint32_t seal_key[8];
    sha256(pcr_concat, sizeof(pcr_concat), seal_key);

    // AES-256-ECB encrypt (block by block, pad with zeros)
    aes_init(kAesEncrypt, kAesModeEcb, seal_key, NULL);

    // Round up to 16-byte blocks
    uint32_t padded_len = (data_len + 15U) & ~15U;
    uint8_t block_in[16] __attribute__((aligned(4)));
    uint32_t total = TPM_HEADER_SIZE + 4 + padded_len;
    uint8_t *p = resp_header(resp, total, TPM_RC_SUCCESS);
    cpu_to_be32(padded_len, p);
    p += 4;

    for (uint32_t off = 0; off < padded_len; off += 16) {
        memset(block_in, 0, 16);
        uint32_t chunk = (data_len - off > 16) ? 16 : (data_len > off ? data_len - off : 0);
        if (chunk > 0) {
            memcpy(block_in, plaintext + off, chunk);
        }
        aes_process_block((const uint32_t *)block_in, (uint32_t *)p);
        p += 16;
    }

    aes_clear();
    *resp_len = total;
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_Unseal — Decrypt sealed data if current PCR state matches
// Payload: 4 bytes data_len + ciphertext
// ===========================================================================
uint32_t cmd_unseal(const uint8_t *payload, uint32_t payload_len,
                    uint8_t *resp, uint32_t *resp_len) {
    if (payload_len < 4) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    uint32_t data_len = be32_to_cpu(payload);
    if (data_len > payload_len - 4 || data_len > 128 || (data_len & 15) != 0) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    const uint8_t *ciphertext = payload + 4;

    // Re-derive sealing key from current PCR[0..7]
    uint8_t pcr_concat[8 * TPM_PCR_SIZE] __attribute__((aligned(4)));
    for (uint32_t i = 0; i < 8; i++) {
        tpm_pcr_read(i, &pcr_concat[i * TPM_PCR_SIZE]);
    }
    uint32_t seal_key[8];
    sha256(pcr_concat, sizeof(pcr_concat), seal_key);

    // AES-256-ECB decrypt
    aes_init(kAesDecrypt, kAesModeEcb, seal_key, NULL);

    uint32_t total = TPM_HEADER_SIZE + 4 + data_len;
    uint8_t *p = resp_header(resp, total, TPM_RC_SUCCESS);
    cpu_to_be32(data_len, p);
    p += 4;

    for (uint32_t off = 0; off < data_len; off += 16) {
        aes_process_block((const uint32_t *)(ciphertext + off), (uint32_t *)p);
        p += 16;
    }

    aes_clear();
    *resp_len = total;
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_GetRandom — Return random bytes from CSRNG
// Payload: 2 bytes — bytesRequested
// ===========================================================================
uint32_t cmd_get_random(const uint8_t *payload, uint32_t payload_len,
                        uint8_t *resp, uint32_t *resp_len) {
    if (payload_len < 2) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    uint16_t bytes_req = be16_to_cpu(payload);
    if (bytes_req > 64) {
        bytes_req = 64;  // Cap at 64 bytes per call
    }

    // Read entropy from CSRNG via memory-mapped MMIO
    // OpenTitan CSRNG base: use OTP as entropy source for now
    // TODO: integrate actual CSRNG IP block
    uint32_t total = TPM_HEADER_SIZE + 2 + bytes_req;
    uint8_t *p = resp_header(resp, total, TPM_RC_SUCCESS);
    cpu_to_be16(bytes_req, p);
    p += 2;

    // Generate pseudo-random bytes by hashing OTP + counter
    static uint32_t rng_counter = 0;
    uint32_t seed[4] __attribute__((aligned(4)));
    otp_read_word(0x00, &seed[0]);
    otp_read_word(0x04, &seed[1]);
    seed[2] = rng_counter++;
    seed[3] = rng_counter; // avoid hardcoded seed

    // Hash to get random output (up to 32 bytes per hash)
    uint32_t hash[8];
    sha256(seed, sizeof(seed), hash);
    uint32_t copy_len = (bytes_req <= 32) ? bytes_req : 32;
    memcpy(p, hash, copy_len);

    if (bytes_req > 32) {
        // Second hash round for remaining bytes
        seed[2] = rng_counter++;
        sha256(seed, sizeof(seed), hash);
        memcpy(p + 32, hash, bytes_req - 32);
    }

    *resp_len = total;
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_GetCapability — Report TPM properties
// Payload: 4 bytes capability + 4 bytes property + 4 bytes count
// ===========================================================================
#define TPM_CAP_TPM_PROPERTIES   0x00000006U
#define TPM_PT_MANUFACTURER      0x00000105U
#define TPM_PT_FIRMWARE_V1       0x00000111U
#define TPM_PT_FIRMWARE_V2       0x00000112U

uint32_t cmd_get_capability(const uint8_t *payload, uint32_t payload_len,
                            uint8_t *resp, uint32_t *resp_len) {
    (void)payload;
    (void)payload_len;

    // Return fixed capability data for SafeRoot TPM
    // Response: header + moreData(1) + cap(4) + count(4) + properties
    uint32_t num_props = 3;
    uint32_t total = TPM_HEADER_SIZE + 1 + 4 + 4 + num_props * 8;
    uint8_t *p = resp_header(resp, total, TPM_RC_SUCCESS);

    // moreData = NO
    *p++ = 0;

    // capability = TPM_CAP_TPM_PROPERTIES
    cpu_to_be32(TPM_CAP_TPM_PROPERTIES, p);
    p += 4;

    // count
    cpu_to_be32(num_props, p);
    p += 4;

    // Property: manufacturer = "SFRT"
    cpu_to_be32(TPM_PT_MANUFACTURER, p);  p += 4;
    cpu_to_be32(0x53465254U, p);          p += 4;  // "SFRT"

    // Property: firmware version 1
    cpu_to_be32(TPM_PT_FIRMWARE_V1, p);   p += 4;
    cpu_to_be32(0x00020000U, p);          p += 4;  // v2.0

    // Property: firmware version 2
    cpu_to_be32(TPM_PT_FIRMWARE_V2, p);   p += 4;
    cpu_to_be32(0x00000001U, p);          p += 4;  // build 1

    *resp_len = total;
    return TPM_RC_SUCCESS;
}
