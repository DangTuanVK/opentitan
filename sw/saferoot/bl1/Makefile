# SafeRoot BL1 (Owner Firmware) Secure Boot — Makefile
# Produces: bl1.vmem (Verilog memory init), bl1.bin, bl1.elf

CROSS   = riscv32-unknown-elf-
CC      = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump
SIZE    = riscv64-unknown-elf-size

CFLAGS  = -march=rv32imc -mabi=ilp32 -Os -Wall -Wextra \
          -ffunction-sections -fdata-sections -nostdlib -nostartfiles \
          -ffreestanding -fno-builtin -I../hal -I../bl0 -I../wdt

LDFLAGS = -T bl1.ld -nostdlib -Wl,--gc-sections -Wl,-Map=bl1.map

# HAL source directory
HAL_DIR = ../hal

# WDT source directory
WDT_DIR = ../wdt

# BL0 shared sources (DICE + verify)
BL0_DIR = ../bl0

# Source files
BL1_SRCS = bl1_main.c bl1_bank.c
BL0_SRCS = $(BL0_DIR)/dice.c $(BL0_DIR)/verify.c
WDT_SRCS = $(WDT_DIR)/wdt.c
HAL_SRCS = $(HAL_DIR)/hmac_drv.c \
           $(HAL_DIR)/otp_drv.c \
           $(HAL_DIR)/flash_drv.c \
           $(HAL_DIR)/uart_drv.c \
           $(HAL_DIR)/gpio_drv.c \
           $(HAL_DIR)/aes_drv.c

SRCS = $(BL1_SRCS) $(BL0_SRCS) $(WDT_SRCS) $(HAL_SRCS)

TARGET = bl1

all: $(TARGET).vmem $(TARGET).bin $(TARGET).dis

$(TARGET).elf: $(SRCS) bl1.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRCS)
	$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).dis: $(TARGET).elf
	$(OBJDUMP) -d -t $< > $@

# Verilog $readmemh format — 32-bit words, one per line
$(TARGET).vmem: $(TARGET).bin
	od -An -tx4 -v -w4 $< | sed 's/^ *//' > $@
	@echo "BL1 image: $@ ($$(wc -l < $@) words, $$(wc -c < $(TARGET).bin) bytes)"

clean:
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).vmem $(TARGET).dis $(TARGET).map

.PHONY: all clean
