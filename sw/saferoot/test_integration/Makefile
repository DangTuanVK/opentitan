# SafeRoot Integration Tests -- Makefile
# Builds a standalone firmware image that exercises all modules together.
# Output: test_integration.vmem (Verilog memory init for Verilator simulation)

CROSS   = riscv32-unknown-elf-
CC      = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump

CFLAGS  = -march=rv32imc -mabi=ilp32 -Os -Wall -Wextra \
          -ffunction-sections -fdata-sections -nostdlib -nostartfiles \
          -ffreestanding -fno-builtin \
          -I../hal -I../bl0 -I../tpm -I../se -I../wdt -I../ota -I../test
LDFLAGS = -T ../rom/rom.ld -nostdlib -Wl,--gc-sections -Wl,-Map=test_integration.map

TARGET  = test_integration

# ---------------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------------
START_S  = ../rom/start.S

# Integration test sources
TEST_SRC = test_integration_main.c \
           test_boot_chain.c \
           test_tpm_pcr.c \
           test_se_crypto.c \
           test_wdt_escalation.c \
           test_ota_flow.c

# HAL driver sources
HAL_SRC  = ../hal/uart_drv.c ../hal/gpio_drv.c ../hal/aes_drv.c \
           ../hal/hmac_drv.c ../hal/flash_drv.c ../hal/otp_drv.c \
           ../hal/spi_drv.c

# Module sources
BL0_SRC  = ../bl0/bl0_main.c ../bl0/verify.c ../bl0/dice.c
TPM_SRC  = ../tpm/tpm.c ../tpm/tpm_cmd.c ../tpm/tpm_pcr.c \
           ../tpm/tpm_nv.c ../tpm/tpm_spi.c
SE_SRC   = ../se/se.c ../se/se_crypto.c ../se/se_keystore.c
WDT_SRC  = ../wdt/wdt.c ../wdt/alert_config.c ../wdt/heartbeat.c
OTA_SRC  = ../ota/ota.c ../ota/ota_spi.c

# ROM utility (memcpy, memset, etc.)
ROM_SRC  = ../rom/util.c

ALL_SRC  = $(START_S) $(TEST_SRC) $(HAL_SRC) $(BL0_SRC) $(TPM_SRC) \
           $(SE_SRC) $(WDT_SRC) $(OTA_SRC) $(ROM_SRC)

# ---------------------------------------------------------------------------
# Build targets
# ---------------------------------------------------------------------------
all: $(TARGET).vmem $(TARGET).bin $(TARGET).dis

$(TARGET).elf: $(ALL_SRC) ../rom/rom.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(ALL_SRC)

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).dis: $(TARGET).elf
	$(OBJDUMP) -d -t $< > $@

# Verilog $readmemh format -- 32-bit words, one per line
$(TARGET).vmem: $(TARGET).bin
	od -An -tx4 -v -w4 $< | sed 's/^ *//' > $@
	@echo "Integration test image: $@ ($$(wc -l < $@) words, $$(wc -c < $(TARGET).bin) bytes)"

clean:
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).vmem $(TARGET).dis $(TARGET).map

.PHONY: all clean
