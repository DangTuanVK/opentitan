// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// tpm_cmd_ext.h — Extended TPM 2.0 command handlers for SafeRoot (spec2)
// Adds 9 commands: NV_DefineSpace, NV_Read, NV_Write, Hash, HMAC,
// Sign, VerifySignature, PolicyPCR, FlushContext.

#ifndef SAFEROOT_TPM_CMD_EXT_H_
#define SAFEROOT_TPM_CMD_EXT_H_

#include "tpm.h"

// All command handlers share the standard signature:
//   payload:     pointer past the 10-byte TPM header
//   payload_len: number of bytes in the payload
//   resp:        response buffer (header space reserved by caller)
//   resp_len:    [out] total response length including header
//   Returns:     TPM_RC_* return code

uint32_t cmd_nv_define_space(const uint8_t *payload, uint32_t payload_len,
                              uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_nv_read(const uint8_t *payload, uint32_t payload_len,
                      uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_nv_write(const uint8_t *payload, uint32_t payload_len,
                       uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_hash(const uint8_t *payload, uint32_t payload_len,
                   uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_hmac(const uint8_t *payload, uint32_t payload_len,
                   uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_sign(const uint8_t *payload, uint32_t payload_len,
                   uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_verify_signature(const uint8_t *payload, uint32_t payload_len,
                               uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_policy_pcr(const uint8_t *payload, uint32_t payload_len,
                         uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_flush_context(const uint8_t *payload, uint32_t payload_len,
                            uint8_t *resp, uint32_t *resp_len);

#endif  // SAFEROOT_TPM_CMD_EXT_H_
