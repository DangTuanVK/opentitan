// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// tpm.c — TPM 2.0 core logic: init, command dispatcher
// Bare-metal C for rv32imc (Ibex CPU).

#include "tpm.h"
#include "tpm_pcr.h"
#include "tpm_nv.h"
#include "tpm_cmd.h"
#include "../hal/uart_drv.h"

// ---------------------------------------------------------------------------
// Global TPM state
// ---------------------------------------------------------------------------
tpm_state_t g_tpm_state = kTpmStateUninitialized;

// ---------------------------------------------------------------------------
// Byte-order helpers (TPM uses big-endian on the wire)
// ---------------------------------------------------------------------------
static uint16_t be16_to_cpu(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0] << 8 | p[1]);
}

static uint32_t be32_to_cpu(const uint8_t *p) {
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 |
           (uint32_t)p[2] << 8  | (uint32_t)p[3];
}

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

// ---------------------------------------------------------------------------
// Build a minimal TPM response header (error response)
// ---------------------------------------------------------------------------
static void tpm_build_error_resp(uint32_t rc, uint8_t *resp,
                                 uint32_t *resp_len) {
    cpu_to_be16(TPM_ST_NO_SESSIONS, &resp[0]);
    cpu_to_be32(TPM_HEADER_SIZE, &resp[2]);
    cpu_to_be32(rc, &resp[6]);
    *resp_len = TPM_HEADER_SIZE;
}

// ---------------------------------------------------------------------------
// tpm_init — Initialize all TPM subsystems
// ---------------------------------------------------------------------------
void tpm_init(void) {
    uart_puts("[TPM] Initializing TPM 2.0 subsystem\r\n");

    tpm_pcr_init();
    tpm_nv_init();

    g_tpm_state = kTpmStateUninitialized;

    uart_puts("[TPM] Init complete — awaiting TPM2_Startup\r\n");
}

// ---------------------------------------------------------------------------
// tpm_process_command — Main command dispatcher
// ---------------------------------------------------------------------------
uint32_t tpm_process_command(const uint8_t *cmd, uint32_t cmd_len,
                             uint8_t *resp, uint32_t *resp_len) {
    // Validate minimum header size
    if (cmd == NULL || resp == NULL || resp_len == NULL ||
        cmd_len < TPM_HEADER_SIZE) {
        if (resp != NULL && resp_len != NULL) {
            tpm_build_error_resp(TPM_RC_FAILURE, resp, resp_len);
        }
        return TPM_RC_FAILURE;
    }

    // Parse header (big-endian)
    uint16_t tag  = be16_to_cpu(&cmd[0]);
    uint32_t size = be32_to_cpu(&cmd[2]);
    uint32_t cc   = be32_to_cpu(&cmd[6]);

    // Validate tag
    if (tag != TPM_ST_NO_SESSIONS && tag != TPM_ST_SESSIONS) {
        tpm_build_error_resp(TPM_RC_BAD_TAG, resp, resp_len);
        return TPM_RC_BAD_TAG;
    }

    // Validate size field matches buffer
    if (size > cmd_len) {
        tpm_build_error_resp(TPM_RC_SIZE, resp, resp_len);
        return TPM_RC_SIZE;
    }

    // Payload starts after header
    const uint8_t *payload     = cmd + TPM_HEADER_SIZE;
    uint32_t       payload_len = size - TPM_HEADER_SIZE;

    // Startup must be the first command (except GetCapability)
    if (g_tpm_state == kTpmStateUninitialized &&
        cc != TPM_CC_Startup && cc != TPM_CC_GetCapability) {
        tpm_build_error_resp(TPM_RC_INITIALIZE, resp, resp_len);
        return TPM_RC_INITIALIZE;
    }

    // Dispatch by command code
    uint32_t rc;
    switch (cc) {
        case TPM_CC_Startup:
            rc = cmd_startup(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_Shutdown:
            rc = cmd_shutdown(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_SelfTest:
            rc = cmd_self_test(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_PCR_Extend:
            rc = cmd_pcr_extend(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_PCR_Read:
            rc = cmd_pcr_read(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_PCR_Reset:
            rc = cmd_pcr_reset(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_Quote:
            rc = cmd_quote(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_CreatePrimary:
            rc = cmd_create_primary(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_Load:
            rc = cmd_load(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_Seal:
            rc = cmd_seal(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_Unseal:
            rc = cmd_unseal(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_GetRandom:
            rc = cmd_get_random(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_GetCapability:
            rc = cmd_get_capability(payload, payload_len, resp, resp_len);
            break;
        default:
            tpm_build_error_resp(TPM_RC_COMMAND_CODE, resp, resp_len);
            rc = TPM_RC_COMMAND_CODE;
            break;
    }

    return rc;
}
