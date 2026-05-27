// Copyright 2026 SafeRoot Project
// SPDX-License-Identifier: Apache-2.0
//
// tpm_cmd.h — TPM 2.0 command handlers for SafeRoot
// Each handler processes a parsed TPM command and writes the response.

#ifndef SAFEROOT_TPM_CMD_H_
#define SAFEROOT_TPM_CMD_H_

#include "tpm.h"

// All command handlers share this signature:
//   payload:     pointer past the 10-byte TPM header
//   payload_len: number of bytes in the payload
//   resp:        response buffer (header already reserved by caller)
//   resp_len:    [out] total response length including header
//   Returns:     TPM_RC_* return code

uint32_t cmd_startup(const uint8_t *payload, uint32_t payload_len,
                     uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_shutdown(const uint8_t *payload, uint32_t payload_len,
                      uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_self_test(const uint8_t *payload, uint32_t payload_len,
                       uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_pcr_extend(const uint8_t *payload, uint32_t payload_len,
                         uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_pcr_read(const uint8_t *payload, uint32_t payload_len,
                      uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_pcr_reset(const uint8_t *payload, uint32_t payload_len,
                       uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_quote(const uint8_t *payload, uint32_t payload_len,
                   uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_create_primary(const uint8_t *payload, uint32_t payload_len,
                            uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_load(const uint8_t *payload, uint32_t payload_len,
                  uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_seal(const uint8_t *payload, uint32_t payload_len,
                  uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_unseal(const uint8_t *payload, uint32_t payload_len,
                    uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_get_random(const uint8_t *payload, uint32_t payload_len,
                        uint8_t *resp, uint32_t *resp_len);

uint32_t cmd_get_capability(const uint8_t *payload, uint32_t payload_len,
                            uint8_t *resp, uint32_t *resp_len);

#endif  // SAFEROOT_TPM_CMD_H_
