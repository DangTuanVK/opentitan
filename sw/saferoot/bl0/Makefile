# SafeRoot BL0 (ROM_EXT) Secure Boot — Makefile
# Produces: bl0.vmem (Verilog memory init), bl0.bin, bl0.elf

CROSS   = riscv32-unknown-elf-
CC      = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump
SIZE    = riscv64-unknown-elf-size

CFLAGS  = -march=rv32imc -mabi=ilp32 -Os -Wall -Wextra \
          -ffunction-sections -fdata-sections -nostdlib -nostartfiles \
          -ffreestanding -fno-builtin -I../hal

LDFLAGS = -T bl0.ld -nostdlib -Wl,--gc-sections -Wl,-Map=bl0.map

# HAL source directory
HAL_DIR = ../hal

# Source files
BL0_SRCS = bl0_main.c dice.c verify.c
HAL_SRCS = $(HAL_DIR)/hmac_drv.c \
           $(HAL_DIR)/otp_drv.c \
           $(HAL_DIR)/flash_drv.c \
           $(HAL_DIR)/uart_drv.c \
           $(HAL_DIR)/gpio_drv.c \
           $(HAL_DIR)/aes_drv.c

SRCS = $(BL0_SRCS) $(HAL_SRCS)

TARGET = bl0

all: $(TARGET).vmem $(TARGET).bin $(TARGET).dis

$(TARGET).elf: $(SRCS) bl0.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRCS)
	$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).dis: $(TARGET).elf
	$(OBJDUMP) -d -t $< > $@

# Verilog $readmemh format — 32-bit words, one per line
$(TARGET).vmem: $(TARGET).bin
	od -An -tx4 -v -w4 $< | sed 's/^ *//' > $@
	@echo "BL0 image: $@ ($$(wc -l < $@) words, $$(wc -c < $(TARGET).bin) bytes)"

clean:
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).vmem $(TARGET).dis $(TARGET).map

.PHONY: all clean
