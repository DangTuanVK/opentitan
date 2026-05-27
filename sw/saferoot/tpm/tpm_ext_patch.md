# TPM 2.0 Extended Commands — Integration Patch Instructions

This document describes the changes needed in existing files to integrate the
9 new TPM 2.0 commands (spec2) into the SafeRoot TPM stack.

**New files added (do NOT modify these instructions — apply them to existing files):**
- `tpm_cmd_ext.h` — prototypes for 9 new command handlers
- `tpm_cmd_ext.c` — implementations
- `tpm_session.h` — session management API
- `tpm_session.c` — session management implementation

---

## 1. `tpm.h` — Add new command code defines

Add after line 36 (`#define TPM_CC_GetCapability`):

```c
#define TPM_CC_NV_DefineSpace   0x0000012AU
#define TPM_CC_NV_Write         0x00000137U
#define TPM_CC_NV_Read          0x0000014EU
#define TPM_CC_HMAC             0x00000155U
#define TPM_CC_Sign             0x0000015DU
#define TPM_CC_FlushContext     0x00000165U
#define TPM_CC_VerifySignature  0x00000177U
#define TPM_CC_Hash             0x0000017DU
#define TPM_CC_PolicyPCR        0x0000017FU
```

Add after line 53 (`#define TPM_RC_NEEDS_TEST`):

```c
#define TPM_RC_HANDLE         0x0000008BU
#define TPM_RC_SIGNATURE      0x0000009BU
#define TPM_RC_POLICY         0x00000126U
#define TPM_RC_NV_DEFINED     0x0000014CU
#define TPM_RC_SESSION        0x000000A5U
```

---

## 2. `tpm.c` — Add dispatcher cases and includes

Add after line 10 (`#include "tpm_cmd.h"`):

```c
#include "tpm_cmd_ext.h"
#include "tpm_session.h"
```

Add in `tpm_init()` after line 61 (`tpm_nv_init();`):

```c
    session_init();
```

Add in the `switch (cc)` block, before the `default:` case (before line 151):

```c
        case TPM_CC_NV_DefineSpace:
            rc = cmd_nv_define_space(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_NV_Read:
            rc = cmd_nv_read(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_NV_Write:
            rc = cmd_nv_write(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_Hash:
            rc = cmd_hash(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_HMAC:
            rc = cmd_hmac(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_Sign:
            rc = cmd_sign(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_VerifySignature:
            rc = cmd_verify_signature(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_PolicyPCR:
            rc = cmd_policy_pcr(payload, payload_len, resp, resp_len);
            break;
        case TPM_CC_FlushContext:
            rc = cmd_flush_context(payload, payload_len, resp, resp_len);
            break;
```

---

## 3. `tpm_cmd.h` — Add include for extended commands

Add after line 56 (`uint32_t cmd_get_capability(...)`), before `#endif`:

```c
// Extended command handlers (spec2)
#include "tpm_cmd_ext.h"
```

Alternatively, include `tpm_cmd_ext.h` directly in `tpm.c` (already covered
in section 2 above), which is the preferred approach to avoid circular
includes.

---

## 4. Build system — Add new source files

If using a Makefile, add to the source list:

```makefile
TPM_SRCS += tpm_cmd_ext.c tpm_session.c
```

If using FuseSoC `.core` file, add under `[filesets]`:

```
files_tpm_ext:
  files:
    - sw/saferoot/tpm/tpm_cmd_ext.c
    - sw/saferoot/tpm/tpm_cmd_ext.h
    - sw/saferoot/tpm/tpm_session.c
    - sw/saferoot/tpm/tpm_session.h
  file_type: cSource
```

---

## Summary of 22 commands after integration

| # | Command            | CC     | Status     |
|---|--------------------|--------|------------|
| 1 | Startup            | 0x144  | existing   |
| 2 | Shutdown           | 0x145  | existing   |
| 3 | SelfTest           | 0x143  | existing   |
| 4 | PCR_Extend         | 0x182  | existing   |
| 5 | PCR_Read           | 0x17E  | existing   |
| 6 | PCR_Reset          | 0x13D  | existing   |
| 7 | Quote              | 0x158  | existing   |
| 8 | CreatePrimary      | 0x131  | existing   |
| 9 | Load               | 0x157  | existing   |
| 10| Seal               | 0x000  | existing   |
| 11| Unseal             | 0x15E  | existing   |
| 12| GetRandom          | 0x17B  | existing   |
| 13| GetCapability      | 0x17A  | existing   |
| 14| NV_DefineSpace     | 0x12A  | **new**    |
| 15| NV_Read            | 0x14E  | **new**    |
| 16| NV_Write           | 0x137  | **new**    |
| 17| Hash               | 0x17D  | **new**    |
| 18| HMAC               | 0x155  | **new**    |
| 19| Sign               | 0x15D  | **new**    |
| 20| VerifySignature    | 0x177  | **new**    |
| 21| PolicyPCR          | 0x17F  | **new**    |
| 22| FlushContext       | 0x165  | **new**    |
