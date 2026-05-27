// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// tpm.h — TPM 2.0 Minimal Implementation for SafeRoot
// Implements a subset of TCG TPM 2.0 spec (13 commands) on Ibex CPU.

#ifndef SAFEROOT_TPM_H_
#define SAFEROOT_TPM_H_

#include <stdint.h>
#include <stdbool.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

// External utility functions (from util.c)
extern void *memcpy(void *dst, const void *src, unsigned int n);
extern void *memset(void *s, int c, unsigned int n);

// ---------------------------------------------------------------------------
// TPM 2.0 Command Codes (TCG Part 2, Table 12)
// ---------------------------------------------------------------------------
#define TPM_CC_Startup        0x00000144U
#define TPM_CC_Shutdown       0x00000145U
#define TPM_CC_SelfTest       0x00000143U
#define TPM_CC_PCR_Extend     0x00000182U
#define TPM_CC_PCR_Read       0x0000017EU
#define TPM_CC_PCR_Reset      0x0000013DU
#define TPM_CC_Quote          0x00000158U
#define TPM_CC_CreatePrimary  0x00000131U
#define TPM_CC_Load           0x00000157U
#define TPM_CC_Seal           0x00000000U  // Mapped via Create with seal flag
#define TPM_CC_Unseal         0x0000015EU
#define TPM_CC_GetRandom      0x0000017BU
#define TPM_CC_GetCapability  0x0000017AU

// ---------------------------------------------------------------------------
// TPM 2.0 Return Codes (TCG Part 2, Table 16)
// ---------------------------------------------------------------------------
#define TPM_RC_SUCCESS        0x00000000U
#define TPM_RC_FAILURE        0x00000101U
#define TPM_RC_BAD_TAG        0x0000001EU
#define TPM_RC_SIZE           0x00000095U
#define TPM_RC_VALUE          0x00000084U
#define TPM_RC_INITIALIZE     0x00000100U
#define TPM_RC_NV_RANGE       0x00000146U
#define TPM_RC_NV_LOCKED      0x00000148U
#define TPM_RC_NV_UNINITIALIZED 0x0000014AU
#define TPM_RC_COMMAND_CODE   0x00000143U
#define TPM_RC_AUTHFAIL       0x0000008EU
#define TPM_RC_PCR            0x00000127U
#define TPM_RC_DISABLED       0x00000120U
#define TPM_RC_NEEDS_TEST     0x00000153U

// ---------------------------------------------------------------------------
// TPM 2.0 Tags
// ---------------------------------------------------------------------------
#define TPM_ST_NO_SESSIONS    0x8001U
#define TPM_ST_SESSIONS       0x8002U

// ---------------------------------------------------------------------------
// TPM Startup Types
// ---------------------------------------------------------------------------
#define TPM_SU_CLEAR          0x0000U
#define TPM_SU_STATE          0x0001U

// ---------------------------------------------------------------------------
// TPM Header (10 bytes)
// ---------------------------------------------------------------------------
typedef struct {
    uint16_t tag;
    uint32_t size;
    uint32_t command_code;
} __attribute__((packed)) tpm_header_t;

#define TPM_HEADER_SIZE  10U

// ---------------------------------------------------------------------------
// PCR Constants
// ---------------------------------------------------------------------------
#define TPM_NUM_PCRS      24U
#define TPM_PCR_SIZE      32U   // SHA-256 digest size in bytes
#define TPM_PCR_RESET_MIN 16U   // PCRs 16-23 are resettable

// ---------------------------------------------------------------------------
// NV Storage Constants
// ---------------------------------------------------------------------------
#define TPM_NV_MAX_INDEX    16U
#define TPM_NV_MAX_SIZE     256U
#define TPM_NV_INDEX_BASE   0x01800000U

// ---------------------------------------------------------------------------
// Session / Handle Constants
// ---------------------------------------------------------------------------
#define TPM_RH_OWNER        0x40000001U
#define TPM_RH_NULL         0x40000007U
#define TPM_RH_ENDORSEMENT  0x4000000BU
#define TPM_RH_PLATFORM     0x4000000CU

// ---------------------------------------------------------------------------
// TPM State
// ---------------------------------------------------------------------------
typedef enum {
    kTpmStateUninitialized = 0,
    kTpmStateStarted       = 1,
    kTpmStateSelfTested    = 2,
} tpm_state_t;

// ---------------------------------------------------------------------------
// Core TPM API
// ---------------------------------------------------------------------------

// Initialize TPM internal state (PCRs, NV, state machine)
void tpm_init(void);

// Process a TPM 2.0 command buffer and produce a response.
// cmd:      pointer to command buffer (TPM header + payload)
// cmd_len:  command buffer length in bytes
// resp:     pointer to response buffer (caller-allocated)
// resp_len: [in] max resp buffer size, [out] actual response size
uint32_t tpm_process_command(const uint8_t *cmd, uint32_t cmd_len,
                             uint8_t *resp, uint32_t *resp_len);

#endif  // SAFEROOT_TPM_H_
