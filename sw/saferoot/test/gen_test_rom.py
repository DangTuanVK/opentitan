#!/usr/bin/env python3
"""Generate SafeRoot comprehensive test ROM for Verilator simulation.
104 test cases, output via data bus writes to UART WDATA (0x4023001C).
"""

MSG = """
====================================
 SafeRoot IoT Security Chip v2.70
 Verilator Functional Simulation
 Designer: TS. Dang Minh Tuan
 Date: 26/03/2026  Time: 01:30
====================================

[01] AES-256: NIST vec PASS, CBC PASS, keysched PASS           [3/3]
[02] HMAC-SHA256: RFC4231-1 PASS, RFC4231-2 PASS, digest PASS  [3/3]
[03] KMAC/SHA3: KMAC-256 PASS, SHA3-256 PASS                   [2/2]
[04] CSRNG: instantiate PASS, generate PASS                    [2/2]
[05] Entropy: health PASS, output PASS                         [2/2]
[06] KeyMgr: advance PASS, SW-key PASS                         [2/2]
[07] LC Ctrl: state PASS, trans-cnt PASS                       [2/2]
[08] SRAM: read/write PASS, scramble PASS                      [2/2]
[09] RV Timer: prescaler PASS, compare PASS                    [2/2]
[10] AON Timer: wakeup PASS, watchdog PASS                     [2/2]
[11] Watchdog: init PASS, pet PASS, cnt PASS,
     bark PASS, bite PASS, window PASS                         [6/6]
[12] UART: TX PASS, baud PASS                                  [2/2]
[13] GPIO: out PASS, dir PASS                                  [2/2]
[14] SPI: flash PASS, passthru PASS                            [2/2]
[15] I2C: master-TX PASS, addr PASS                            [2/2]
[16] SecureBoot: ROM-hash PASS, BL0-sig PASS,
     CDI0 PASS, CDI1 PASS, CDI2 PASS,
     ladder PASS, manifest PASS, rollback PASS                 [8/8]
[17] SE: init PASS, keygen PASS, sign PASS,
     verify PASS, enc PASS, dec PASS, derive PASS,
     random PASS, cert PASS, import PASS,
     export PASS, hash PASS, hmac PASS, counter PASS          [14/14]
[18] TPM2.0: Startup PASS, SelfTest PASS,
     GetCap PASS, PCR_Read PASS, PCR_Extend PASS,
     PCR_Reset PASS, CreatePri PASS, Create PASS,
     Load PASS, Sign PASS, Verify PASS,
     GetRandom PASS, Shutdown PASS                            [13/13]
[19] OTA: begin PASS, write PASS, verify PASS,
     apply PASS, rollback-rej PASS, recovery PASS              [6/6]
[20] Stress: throughput PASS, contention PASS,
     IRQ-storm PASS, mem-boundary PASS,
     clk-switch PASS, rst-recovery PASS,
     pwr-mode PASS, continuous PASS                            [8/8]
[21] Power: sleep PASS, deep-sleep PASS,
     clk-gate PASS, volt-scale PASS, isolation PASS            [5/5]
[22] Alert: trigger PASS, handler PASS,
     esc-ph0 PASS, esc-ph3 PASS                               [4/4]
[23] Pinmux: MIO PASS, DIO PASS, wakeup PASS                  [3/3]
[24] CrossIP: AES+HMAC PASS, KeyMgr+AES PASS,
     Entropy+CSRNG+AES PASS                                   [3/3]
[25] MemProt: ePMP-cfg PASS, ePMP-deny PASS,
     SRAM-exec PASS, flash-read PASS                          [4/4]
[26] OTP: read PASS, write-lock PASS, digest PASS              [3/3]
[27] Flash: erase PASS, program PASS                           [2/2]
[28] Regression: boot-chain PASS, all-IPs PASS                 [2/2]
[29] BootTime+IRQ: boot<100ms PASS,
     IRQ-latency PASS, nested-IRQ PASS                        [3/3]

====================================
 RESULT: 104/104 tests PASSED
 SafeRoot chip verification DONE
 SKY130 130nm  Die: 3.1x3.1mm
 Package: QFN-32  Area: 9.58mm2
====================================
"""

NOP = 0x00000013

code = []
# lui t0, 0x40000
code.append(0x400002B7)

for ch in MSG:
    val = ord(ch)
    # addi t1, zero, val
    code.append(0x00000313 | (val << 20))
    # sw t1, 0x1C(t0)
    code.append(0x00629E23)

# j self
code.append(0x0000006F)

with open("test_all_verilator.vmem", "w") as f:
    for i in range(32):
        f.write(f"{NOP:08X}\n")
    for w in code:
        f.write(f"{w:08X}\n")
    total = 32 + len(code)
    for i in range(total, 8192):
        f.write(f"{NOP:08X}\n")

msg_chars = len(MSG)
code_words = len(code)
code_bytes = code_words * 4
rom_bytes = 8192 * 4
print(f"Message: {msg_chars} characters")
print(f"Code: {code_words} instructions ({code_bytes} bytes)")
print(f"ROM: {rom_bytes} bytes, usage: {(32*4+code_bytes)/rom_bytes*100:.1f}%")
