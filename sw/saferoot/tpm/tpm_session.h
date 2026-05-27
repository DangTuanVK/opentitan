// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// tpm_session.h — Simple session management for PolicyPCR authorization
// Supports up to TPM_MAX_SESSIONS active sessions.

#ifndef SAFEROOT_TPM_SESSION_H_
#define SAFEROOT_TPM_SESSION_H_

#include "tpm.h"

#define TPM_MAX_SESSIONS    4U
#define TPM_SESSION_HANDLE_BASE  0x03000000U

// Session state
typedef struct {
    uint32_t handle;                     // 0 = unused slot
    uint8_t  policy_digest[TPM_PCR_SIZE]; // Expected PCR composite digest
    bool     is_authorized;              // Set true after PolicyPCR succeeds
} tpm_session_t;

// Initialize session table (clear all sessions)
void session_init(void);

// Create a new session, returns handle via *out_handle.
// Returns TPM_RC_SUCCESS or TPM_RC_FAILURE if table full.
uint32_t session_create(uint32_t *out_handle);

// Look up a session by handle.
// Returns pointer to session, or NULL if not found.
tpm_session_t *session_get(uint32_t handle);

// Flush (invalidate) a session by handle.
// Returns TPM_RC_SUCCESS or TPM_RC_VALUE if not found.
uint32_t session_flush(uint32_t handle);

#endif  // SAFEROOT_TPM_SESSION_H_
