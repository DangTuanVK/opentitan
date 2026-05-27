// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// tpm_session.c — Simple session management for PolicyPCR authorization
// Bare-metal C for rv32imc (Ibex CPU).

#include "tpm_session.h"

// ---------------------------------------------------------------------------
// Session table
// ---------------------------------------------------------------------------
static tpm_session_t g_sessions[TPM_MAX_SESSIONS];
static uint32_t      g_next_session_seq = 1U;

// ---------------------------------------------------------------------------
// session_init — Clear all session slots
// ---------------------------------------------------------------------------
void session_init(void) {
    for (uint32_t i = 0; i < TPM_MAX_SESSIONS; i++) {
        g_sessions[i].handle = 0;
        g_sessions[i].is_authorized = false;
        memset(g_sessions[i].policy_digest, 0, TPM_PCR_SIZE);
    }
    g_next_session_seq = 1U;
}

// ---------------------------------------------------------------------------
// session_create — Allocate a new session slot
// ---------------------------------------------------------------------------
uint32_t session_create(uint32_t *out_handle) {
    for (uint32_t i = 0; i < TPM_MAX_SESSIONS; i++) {
        if (g_sessions[i].handle == 0) {
            uint32_t h = TPM_SESSION_HANDLE_BASE | g_next_session_seq++;
            g_sessions[i].handle = h;
            g_sessions[i].is_authorized = false;
            memset(g_sessions[i].policy_digest, 0, TPM_PCR_SIZE);
            if (out_handle != NULL) {
                *out_handle = h;
            }
            return TPM_RC_SUCCESS;
        }
    }
    return TPM_RC_FAILURE;
}

// ---------------------------------------------------------------------------
// session_get — Look up session by handle
// ---------------------------------------------------------------------------
tpm_session_t *session_get(uint32_t handle) {
    for (uint32_t i = 0; i < TPM_MAX_SESSIONS; i++) {
        if (g_sessions[i].handle == handle) {
            return &g_sessions[i];
        }
    }
    return NULL;
}

// ---------------------------------------------------------------------------
// session_flush — Invalidate a session, zero its memory
// ---------------------------------------------------------------------------
uint32_t session_flush(uint32_t handle) {
    for (uint32_t i = 0; i < TPM_MAX_SESSIONS; i++) {
        if (g_sessions[i].handle == handle) {
            g_sessions[i].handle = 0;
            g_sessions[i].is_authorized = false;
            memset(g_sessions[i].policy_digest, 0, TPM_PCR_SIZE);
            return TPM_RC_SUCCESS;
        }
    }
    return TPM_RC_VALUE;
}
