// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// tpm_cmd_ext.c — Extended TPM 2.0 command handlers for SafeRoot (spec2)
// 9 new commands: NV_DefineSpace, NV_Read, NV_Write, Hash, HMAC,
// Sign, VerifySignature, PolicyPCR, FlushContext.
// Bare-metal C for rv32imc (Ibex CPU).

#include "tpm_cmd_ext.h"
#include "tpm_pcr.h"
#include "tpm_nv.h"
#include "tpm_session.h"
#include "../hal/hmac_drv.h"
#include "../hal/uart_drv.h"

// ---------------------------------------------------------------------------
// Access global TPM state (defined in tpm.c)
// ---------------------------------------------------------------------------
extern tpm_state_t g_tpm_state;

// ---------------------------------------------------------------------------
// Byte-order helpers (kept local to avoid cross-unit dependencies)
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

// Shared helpers — defined in tpm_cmd.c, declared here as extern
extern uint32_t be32_to_cpu(const uint8_t *p);
extern uint8_t *resp_header(uint8_t *resp, uint32_t total_size, uint32_t rc);
extern void resp_simple(uint32_t rc, uint8_t *resp, uint32_t *resp_len);

// ---------------------------------------------------------------------------
// NV metadata: per-index attributes stored in SRAM
// ---------------------------------------------------------------------------
#define NV_ATTR_DEFINED     0x01U   // Index has been defined
#define NV_ATTR_WRITTEN     0x02U   // Index has been written at least once
#define NV_ATTR_OWNER_READ  0x04U   // Owner authorization required for read
#define NV_ATTR_OWNER_WRITE 0x08U   // Owner authorization required for write
#define NV_ATTR_POLICY_WRITE 0x10U  // Policy authorization required for write

typedef struct {
    uint32_t attributes;
    uint16_t data_size;     // Defined size of this NV index
    uint8_t  auth[TPM_PCR_SIZE]; // Authorization value (HMAC key)
} nv_meta_t;

static nv_meta_t g_nv_meta[TPM_NV_MAX_INDEX];

// ---------------------------------------------------------------------------
// Loaded key slot (simplified: single active key)
// ---------------------------------------------------------------------------
#define KEY_HANDLE_BASE     0x80000000U
#define MAX_LOADED_KEYS     4U

typedef struct {
    uint32_t handle;                 // 0 = unused
    uint8_t  priv_key[32];           // Private key material (ECC P-256 scalar)
    uint8_t  pub_key[64];            // Public key (x,y coordinates)
    bool     occupied;
} loaded_key_t;

static loaded_key_t g_loaded_keys[MAX_LOADED_KEYS];

// ===========================================================================
// TPM2_NV_DefineSpace (CC=0x12A)
// Payload: 4 bytes auth_handle + 4 bytes nv_index + 2 bytes data_size +
//          4 bytes attributes + 32 bytes auth_value
// ===========================================================================
uint32_t cmd_nv_define_space(const uint8_t *payload, uint32_t payload_len,
                              uint8_t *resp, uint32_t *resp_len) {
    // Minimum payload: auth_handle(4) + nv_index(4) + data_size(2) + attrs(4)
    if (payload_len < 14) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    uint32_t auth_handle = be32_to_cpu(&payload[0]);
    uint32_t nv_index    = be32_to_cpu(&payload[4]);
    uint16_t data_size   = be16_to_cpu(&payload[8]);
    uint32_t attributes  = be32_to_cpu(&payload[10]);

    (void)auth_handle;  // Simplified: no hierarchical auth check

    // Convert NV index to internal slot
    uint32_t slot;
    if (nv_index >= TPM_NV_INDEX_BASE) {
        slot = nv_index - TPM_NV_INDEX_BASE;
    } else {
        slot = nv_index;
    }

    if (slot >= TPM_NV_MAX_INDEX) {
        resp_simple(TPM_RC_NV_RANGE, resp, resp_len);
        return TPM_RC_NV_RANGE;
    }

    if (data_size == 0 || data_size > TPM_NV_MAX_SIZE) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    // Check if already defined
    if (g_nv_meta[slot].attributes & NV_ATTR_DEFINED) {
        resp_simple(TPM_RC_NV_RANGE, resp, resp_len);
        return TPM_RC_NV_RANGE;
    }

    // Store metadata
    g_nv_meta[slot].attributes = NV_ATTR_DEFINED | (attributes & 0x1CU);
    g_nv_meta[slot].data_size  = data_size;
    memset(g_nv_meta[slot].auth, 0, TPM_PCR_SIZE);

    // Copy auth value if provided
    if (payload_len >= 14 + TPM_PCR_SIZE) {
        memcpy(g_nv_meta[slot].auth, &payload[14], TPM_PCR_SIZE);
    }

    uart_puts("[TPM] NV_DefineSpace: index defined\r\n");
    resp_simple(TPM_RC_SUCCESS, resp, resp_len);
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_NV_Read (CC=0x14E)
// Payload: 4 bytes auth_handle + 4 bytes nv_index + 2 bytes size +
//          2 bytes offset
// Response: header + 2 bytes size + data
// ===========================================================================
uint32_t cmd_nv_read(const uint8_t *payload, uint32_t payload_len,
                      uint8_t *resp, uint32_t *resp_len) {
    if (payload_len < 12) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    uint32_t auth_handle = be32_to_cpu(&payload[0]);
    uint32_t nv_index    = be32_to_cpu(&payload[4]);
    uint16_t read_size   = be16_to_cpu(&payload[8]);
    uint16_t offset      = be16_to_cpu(&payload[10]);

    (void)auth_handle;
    (void)offset;  // Simplified: always read from start

    // Convert NV index to internal slot
    uint32_t slot;
    if (nv_index >= TPM_NV_INDEX_BASE) {
        slot = nv_index - TPM_NV_INDEX_BASE;
    } else {
        slot = nv_index;
    }

    if (slot >= TPM_NV_MAX_INDEX) {
        resp_simple(TPM_RC_NV_RANGE, resp, resp_len);
        return TPM_RC_NV_RANGE;
    }

    // Check defined
    if (!(g_nv_meta[slot].attributes & NV_ATTR_DEFINED)) {
        resp_simple(TPM_RC_NV_UNINITIALIZED, resp, resp_len);
        return TPM_RC_NV_UNINITIALIZED;
    }

    // Check written
    if (!(g_nv_meta[slot].attributes & NV_ATTR_WRITTEN)) {
        resp_simple(TPM_RC_NV_UNINITIALIZED, resp, resp_len);
        return TPM_RC_NV_UNINITIALIZED;
    }

    // Clamp read size to defined size
    if (read_size > g_nv_meta[slot].data_size) {
        read_size = g_nv_meta[slot].data_size;
    }

    // Read from NV storage
    uint8_t nv_data[TPM_NV_MAX_SIZE];
    uint32_t rc = tpm_nv_read(slot, nv_data, read_size);
    if (rc != TPM_RC_SUCCESS) {
        resp_simple(rc, resp, resp_len);
        return rc;
    }

    // Build response: header + size(2) + data
    uint32_t total = TPM_HEADER_SIZE + 2 + read_size;
    uint8_t *p = resp_header(resp, total, TPM_RC_SUCCESS);
    cpu_to_be16(read_size, p);
    p += 2;
    memcpy(p, nv_data, read_size);

    *resp_len = total;
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_NV_Write (CC=0x137)
// Payload: 4 bytes auth_handle + 4 bytes nv_index + 2 bytes size + data +
//          2 bytes offset
// ===========================================================================
uint32_t cmd_nv_write(const uint8_t *payload, uint32_t payload_len,
                       uint8_t *resp, uint32_t *resp_len) {
    if (payload_len < 12) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    uint32_t auth_handle = be32_to_cpu(&payload[0]);
    uint32_t nv_index    = be32_to_cpu(&payload[4]);
    uint16_t write_size  = be16_to_cpu(&payload[8]);

    (void)auth_handle;

    // Validate write data is present
    if (payload_len < 10U + write_size) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    const uint8_t *write_data = &payload[10];

    // Convert NV index to internal slot
    uint32_t slot;
    if (nv_index >= TPM_NV_INDEX_BASE) {
        slot = nv_index - TPM_NV_INDEX_BASE;
    } else {
        slot = nv_index;
    }

    if (slot >= TPM_NV_MAX_INDEX) {
        resp_simple(TPM_RC_NV_RANGE, resp, resp_len);
        return TPM_RC_NV_RANGE;
    }

    // Check defined
    if (!(g_nv_meta[slot].attributes & NV_ATTR_DEFINED)) {
        resp_simple(TPM_RC_NV_UNINITIALIZED, resp, resp_len);
        return TPM_RC_NV_UNINITIALIZED;
    }

    // Check size fits
    if (write_size > g_nv_meta[slot].data_size) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    // Write to NV storage
    uint32_t rc = tpm_nv_write(slot, write_data, write_size);
    if (rc != TPM_RC_SUCCESS) {
        resp_simple(rc, resp, resp_len);
        return rc;
    }

    g_nv_meta[slot].attributes |= NV_ATTR_WRITTEN;

    uart_puts("[TPM] NV_Write: data written\r\n");
    resp_simple(TPM_RC_SUCCESS, resp, resp_len);
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_Hash (CC=0x17D)
// Payload: 2 bytes data_len + data + 4 bytes hash_alg (ignored, always SHA-256)
// Response: header + 2 bytes digest_len + 32 bytes digest
// ===========================================================================
uint32_t cmd_hash(const uint8_t *payload, uint32_t payload_len,
                   uint8_t *resp, uint32_t *resp_len) {
    if (payload_len < 2) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    uint16_t data_len = be16_to_cpu(&payload[0]);
    if (payload_len < 2U + data_len) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    const uint8_t *data = &payload[2];

    // Use HMAC HW in SHA-256 mode (key=NULL)
    uint32_t digest[8];
    hal_status_t st = hmac_init(NULL);
    if (st != kHalOk) {
        resp_simple(TPM_RC_FAILURE, resp, resp_len);
        return TPM_RC_FAILURE;
    }

    // Feed data in 4-byte aligned chunks
    uint32_t aligned_len = data_len & ~3U;
    if (aligned_len > 0) {
        // Copy to aligned buffer for HW requirement
        uint8_t aligned_buf[512] __attribute__((aligned(4)));
        uint32_t chunk = (aligned_len > sizeof(aligned_buf))
                         ? sizeof(aligned_buf) : aligned_len;
        memcpy(aligned_buf, data, chunk);
        hmac_update(aligned_buf, chunk);
        if (aligned_len > chunk) {
            memcpy(aligned_buf, data + chunk, aligned_len - chunk);
            hmac_update(aligned_buf, aligned_len - chunk);
        }
    }

    // Handle remaining bytes (pad to 4-byte boundary)
    uint32_t remainder = data_len - aligned_len;
    if (remainder > 0) {
        uint8_t pad_buf[4] __attribute__((aligned(4)));
        memset(pad_buf, 0, 4);
        memcpy(pad_buf, data + aligned_len, remainder);
        hmac_update(pad_buf, 4);
    }

    st = hmac_final(digest);
    if (st != kHalOk) {
        resp_simple(TPM_RC_FAILURE, resp, resp_len);
        return TPM_RC_FAILURE;
    }

    // Build response: header + digest_size(2) + digest(32)
    uint32_t total = TPM_HEADER_SIZE + 2 + TPM_PCR_SIZE;
    uint8_t *p = resp_header(resp, total, TPM_RC_SUCCESS);
    cpu_to_be16(TPM_PCR_SIZE, p);
    p += 2;
    memcpy(p, digest, TPM_PCR_SIZE);

    *resp_len = total;
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_HMAC (CC=0x155)
// Payload: 4 bytes key_handle + 2 bytes data_len + data
// Response: header + 2 bytes digest_len + 32 bytes HMAC digest
// ===========================================================================
uint32_t cmd_hmac(const uint8_t *payload, uint32_t payload_len,
                   uint8_t *resp, uint32_t *resp_len) {
    if (payload_len < 6) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    uint32_t key_handle = be32_to_cpu(&payload[0]);
    uint16_t data_len   = be16_to_cpu(&payload[4]);

    if (payload_len < 6U + data_len) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    const uint8_t *data = &payload[6];

    // Look up key handle for HMAC key material
    loaded_key_t *key = NULL;
    for (uint32_t i = 0; i < MAX_LOADED_KEYS; i++) {
        if (g_loaded_keys[i].occupied &&
            g_loaded_keys[i].handle == key_handle) {
            key = &g_loaded_keys[i];
            break;
        }
    }

    // Use loaded key as HMAC key, or fall back to zeros
    uint32_t hmac_key[8];
    if (key != NULL) {
        memcpy(hmac_key, key->priv_key, 32);
    } else {
        memset(hmac_key, 0, 32);
        uart_puts("[TPM] HMAC: key handle not found, using zero key\r\n");
    }

    // Initialize HMAC with key
    hal_status_t st = hmac_init(hmac_key);
    if (st != kHalOk) {
        resp_simple(TPM_RC_FAILURE, resp, resp_len);
        return TPM_RC_FAILURE;
    }

    // Feed data (aligned copy for HW)
    uint8_t aligned_buf[512] __attribute__((aligned(4)));
    uint32_t processed = 0;
    while (processed < data_len) {
        uint32_t chunk = data_len - processed;
        if (chunk > sizeof(aligned_buf)) {
            chunk = sizeof(aligned_buf);
        }
        // Round up to 4-byte boundary for HW
        uint32_t hw_len = (chunk + 3U) & ~3U;
        memset(aligned_buf, 0, hw_len);
        memcpy(aligned_buf, data + processed, chunk);
        hmac_update(aligned_buf, hw_len);
        processed += chunk;
    }

    uint32_t digest[8];
    st = hmac_final(digest);
    if (st != kHalOk) {
        resp_simple(TPM_RC_FAILURE, resp, resp_len);
        return TPM_RC_FAILURE;
    }

    // Wipe key from stack
    memset(hmac_key, 0, sizeof(hmac_key));

    // Build response: header + digest_size(2) + digest(32)
    uint32_t total = TPM_HEADER_SIZE + 2 + TPM_PCR_SIZE;
    uint8_t *p = resp_header(resp, total, TPM_RC_SUCCESS);
    cpu_to_be16(TPM_PCR_SIZE, p);
    p += 2;
    memcpy(p, digest, TPM_PCR_SIZE);

    *resp_len = total;
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_Sign (CC=0x15D)
// Payload: 4 bytes key_handle + 32 bytes digest
// Response: header + 2 bytes sig_size + 64 bytes ECDSA signature (r,s)
// Uses ecdsa_p256_sign from ../crypto/ (stub if unavailable).
// ===========================================================================
uint32_t cmd_sign(const uint8_t *payload, uint32_t payload_len,
                   uint8_t *resp, uint32_t *resp_len) {
    if (payload_len < 4 + TPM_PCR_SIZE) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    uint32_t key_handle = be32_to_cpu(&payload[0]);
    const uint8_t *digest = &payload[4];

    // Look up loaded key
    loaded_key_t *key = NULL;
    for (uint32_t i = 0; i < MAX_LOADED_KEYS; i++) {
        if (g_loaded_keys[i].occupied &&
            g_loaded_keys[i].handle == key_handle) {
            key = &g_loaded_keys[i];
            break;
        }
    }

    if (key == NULL) {
        uart_puts("[TPM] Sign: key handle not found\r\n");
        resp_simple(TPM_RC_VALUE, resp, resp_len);
        return TPM_RC_VALUE;
    }

    // Build response: header + sig_scheme(2) + signature(64)
    uint32_t total = TPM_HEADER_SIZE + 2 + 64;
    uint8_t *p = resp_header(resp, total, TPM_RC_SUCCESS);

    // Signature scheme: ECDSA (0x0018 = TPM_ALG_ECDSA)
    cpu_to_be16(0x0018U, p);
    p += 2;

    // TODO: call ecdsa_p256_sign(key->priv_key, digest, sig_r, sig_s)
    // when OTBN crypto library is integrated.
    // For now: produce deterministic stub signature = HMAC(priv_key, digest)
    uint32_t stub_sig[8];
    hal_status_t st = hmac_init((const uint32_t *)key->priv_key);
    if (st == kHalOk) {
        uint8_t digest_aligned[32] __attribute__((aligned(4)));
        memcpy(digest_aligned, digest, 32);
        hmac_update(digest_aligned, 32);
        hmac_final(stub_sig);
    } else {
        memset(stub_sig, 0xAA, 32);
    }

    // r = HMAC result, s = bitwise complement (stub)
    memcpy(p, stub_sig, 32);       // r
    for (uint32_t i = 0; i < 8; i++) {
        stub_sig[i] = ~stub_sig[i];
    }
    memcpy(p + 32, stub_sig, 32);  // s

    // Wipe sensitive data
    memset(stub_sig, 0, sizeof(stub_sig));

    *resp_len = total;
    uart_puts("[TPM] Sign: ECDSA is STUB (HMAC-derived)\r\n");
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_VerifySignature (CC=0x177)
// Payload: 4 bytes key_handle + 32 bytes digest + 2 bytes sig_scheme +
//          64 bytes signature (r,s)
// Response: header only (success/failure)
// ===========================================================================
uint32_t cmd_verify_signature(const uint8_t *payload, uint32_t payload_len,
                               uint8_t *resp, uint32_t *resp_len) {
    if (payload_len < 4 + TPM_PCR_SIZE + 2 + 64) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    uint32_t key_handle      = be32_to_cpu(&payload[0]);
    const uint8_t *digest    = &payload[4];
    // uint16_t sig_scheme   = be16_to_cpu(&payload[36]);  // ignored for now
    const uint8_t *signature = &payload[38];

    // Look up loaded key
    loaded_key_t *key = NULL;
    for (uint32_t i = 0; i < MAX_LOADED_KEYS; i++) {
        if (g_loaded_keys[i].occupied &&
            g_loaded_keys[i].handle == key_handle) {
            key = &g_loaded_keys[i];
            break;
        }
    }

    if (key == NULL) {
        uart_puts("[TPM] VerifySignature: key handle not found\r\n");
        resp_simple(TPM_RC_VALUE, resp, resp_len);
        return TPM_RC_VALUE;
    }

    // TODO: call ecdsa_p256_verify(key->pub_key, digest, sig_r, sig_s)
    // when OTBN crypto library is integrated.
    // Stub verification: recompute HMAC-based signature and compare r part
    uint32_t expected_r[8];
    hal_status_t st = hmac_init((const uint32_t *)key->priv_key);
    if (st == kHalOk) {
        uint8_t digest_aligned[32] __attribute__((aligned(4)));
        memcpy(digest_aligned, digest, 32);
        hmac_update(digest_aligned, 32);
        hmac_final(expected_r);
    } else {
        memset(expected_r, 0xAA, 32);
    }

    // Constant-time compare first 32 bytes (r component)
    uint32_t diff = 0;
    const uint8_t *expected = (const uint8_t *)expected_r;
    for (uint32_t i = 0; i < 32; i++) {
        diff |= expected[i] ^ signature[i];
    }

    memset(expected_r, 0, sizeof(expected_r));

    if (diff != 0) {
        uart_puts("[TPM] VerifySignature: FAIL\r\n");
        resp_simple(TPM_RC_AUTHFAIL, resp, resp_len);
        return TPM_RC_AUTHFAIL;
    }

    uart_puts("[TPM] VerifySignature: OK (stub)\r\n");

    // Return a validation ticket: header + tag(2) + hierarchy(4)
    uint32_t total = TPM_HEADER_SIZE + 2 + 4;
    uint8_t *p = resp_header(resp, total, TPM_RC_SUCCESS);
    cpu_to_be16(0x8021U, p);           // TPM_ST_VERIFIED
    p += 2;
    cpu_to_be32(TPM_RH_OWNER, p);     // hierarchy

    *resp_len = total;
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_PolicyPCR (CC=0x17F)
// Payload: 4 bytes session_handle + 32 bytes pcr_digest +
//          4 bytes pcr_selection (simplified: bitmask of PCR indices 0-23)
// Compares current PCR composite against provided digest.
// ===========================================================================
uint32_t cmd_policy_pcr(const uint8_t *payload, uint32_t payload_len,
                         uint8_t *resp, uint32_t *resp_len) {
    if (payload_len < 4 + TPM_PCR_SIZE + 4) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    uint32_t session_handle   = be32_to_cpu(&payload[0]);
    const uint8_t *pcr_digest = &payload[4];
    uint32_t pcr_select       = be32_to_cpu(&payload[4 + TPM_PCR_SIZE]);

    // Look up session
    tpm_session_t *sess = session_get(session_handle);
    if (sess == NULL) {
        // Auto-create session if handle looks like a session handle
        uint32_t new_handle;
        uint32_t rc = session_create(&new_handle);
        if (rc != TPM_RC_SUCCESS) {
            resp_simple(TPM_RC_FAILURE, resp, resp_len);
            return TPM_RC_FAILURE;
        }
        sess = session_get(new_handle);
        if (sess == NULL) {
            resp_simple(TPM_RC_FAILURE, resp, resp_len);
            return TPM_RC_FAILURE;
        }
    }

    // Compute composite hash of selected PCRs
    uint8_t pcr_concat[TPM_NUM_PCRS * TPM_PCR_SIZE] __attribute__((aligned(4)));
    uint32_t concat_len = 0;

    for (uint32_t i = 0; i < TPM_NUM_PCRS; i++) {
        if (pcr_select & (1U << i)) {
            tpm_pcr_read(i, &pcr_concat[concat_len]);
            concat_len += TPM_PCR_SIZE;
        }
    }

    uint32_t computed_digest[8];
    if (concat_len > 0) {
        sha256(pcr_concat, concat_len, computed_digest);
    } else {
        memset(computed_digest, 0, TPM_PCR_SIZE);
    }

    // Constant-time comparison
    uint32_t diff = 0;
    const uint8_t *computed = (const uint8_t *)computed_digest;
    for (uint32_t i = 0; i < TPM_PCR_SIZE; i++) {
        diff |= computed[i] ^ pcr_digest[i];
    }

    if (diff != 0) {
        sess->is_authorized = false;
        uart_puts("[TPM] PolicyPCR: digest MISMATCH\r\n");
        resp_simple(TPM_RC_PCR, resp, resp_len);
        return TPM_RC_PCR;
    }

    // Policy satisfied — store digest and mark authorized
    memcpy(sess->policy_digest, pcr_digest, TPM_PCR_SIZE);
    sess->is_authorized = true;

    uart_puts("[TPM] PolicyPCR: session authorized\r\n");
    resp_simple(TPM_RC_SUCCESS, resp, resp_len);
    return TPM_RC_SUCCESS;
}

// ===========================================================================
// TPM2_FlushContext (CC=0x165)
// Payload: 4 bytes flush_handle
// Invalidates loaded key or session, zeroes memory.
// ===========================================================================
uint32_t cmd_flush_context(const uint8_t *payload, uint32_t payload_len,
                            uint8_t *resp, uint32_t *resp_len) {
    if (payload_len < 4) {
        resp_simple(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    uint32_t flush_handle = be32_to_cpu(&payload[0]);

    // Try flushing as a loaded key handle
    if ((flush_handle & 0xFF000000U) == KEY_HANDLE_BASE) {
        for (uint32_t i = 0; i < MAX_LOADED_KEYS; i++) {
            if (g_loaded_keys[i].occupied &&
                g_loaded_keys[i].handle == flush_handle) {
                // Zero sensitive key material
                memset(g_loaded_keys[i].priv_key, 0, 32);
                memset(g_loaded_keys[i].pub_key, 0, 64);
                g_loaded_keys[i].handle = 0;
                g_loaded_keys[i].occupied = false;
                uart_puts("[TPM] FlushContext: key flushed\r\n");
                resp_simple(TPM_RC_SUCCESS, resp, resp_len);
                return TPM_RC_SUCCESS;
            }
        }
    }

    // Try flushing as a session handle
    if ((flush_handle & 0xFF000000U) == TPM_SESSION_HANDLE_BASE) {
        uint32_t rc = session_flush(flush_handle);
        if (rc == TPM_RC_SUCCESS) {
            uart_puts("[TPM] FlushContext: session flushed\r\n");
            resp_simple(TPM_RC_SUCCESS, resp, resp_len);
            return TPM_RC_SUCCESS;
        }
    }

    // Handle not found
    uart_puts("[TPM] FlushContext: handle not found\r\n");
    resp_simple(TPM_RC_VALUE, resp, resp_len);
    return TPM_RC_VALUE;
}
