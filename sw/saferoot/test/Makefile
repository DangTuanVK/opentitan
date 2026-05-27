# SafeRoot HAL Unit Tests — Makefile
# Produces: test.vmem (Verilog memory init file for Verilator simulation)

CROSS   = riscv32-unknown-elf-
CC      = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump

CFLAGS  = -march=rv32imc -mabi=ilp32 -Os -Wall -Wextra \
          -ffunction-sections -fdata-sections -nostdlib -nostartfiles \
          -ffreestanding -fno-builtin \
          -I../hal -I../wdt
LDFLAGS = -T ../rom/rom.ld -nostdlib -Wl,--gc-sections -Wl,-Map=test.map

TARGET  = test

# ---------------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------------
START_S  = ../rom/start.S

# Test sources
TEST_SRC = test_main.c test_uart.c test_gpio.c test_crypto.c test_wdt.c

# HAL driver sources
HAL_SRC  = ../hal/uart_drv.c ../hal/gpio_drv.c ../hal/aes_drv.c \
           ../hal/hmac_drv.c

# WDT sources
WDT_SRC  = ../wdt/wdt.c

ALL_SRC  = $(START_S) $(TEST_SRC) $(HAL_SRC) $(WDT_SRC)

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

# Verilog $readmemh format — 32-bit words, one per line
$(TARGET).vmem: $(TARGET).bin
	od -An -tx4 -v -w4 $< | sed 's/^ *//' > $@
	@echo "Test image: $@ ($$(wc -l < $@) words, $$(wc -c < $(TARGET).bin) bytes)"

clean:
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).vmem $(TARGET).dis $(TARGET).map

.PHONY: all clean
