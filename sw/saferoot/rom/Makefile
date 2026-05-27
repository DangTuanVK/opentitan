# SafeRoot Boot ROM — Makefile
# Produces: rom.vmem (Verilog memory init file for simulation)

CROSS   = riscv32-unknown-elf-
CC      = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump
SIZE    = riscv64-unknown-elf-size

CFLAGS  = -march=rv32imc -mabi=ilp32 -Os -Wall -Wextra \
          -ffunction-sections -fdata-sections -nostdlib -nostartfiles \
          -ffreestanding -fno-builtin -I../hal
LDFLAGS = -T rom.ld -nostdlib -Wl,--gc-sections -Wl,-Map=rom.map

TARGET  = rom

all: $(TARGET).vmem $(TARGET).bin $(TARGET).dis

HAL_SRC = ../hal/uart_drv.c ../hal/gpio_drv.c

$(TARGET).elf: start.S main.c util.c $(HAL_SRC) rom.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ start.S main.c util.c $(HAL_SRC)
	$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).dis: $(TARGET).elf
	$(OBJDUMP) -d -t $< > $@

# Verilog $readmemh format — 32-bit words, one per line
$(TARGET).vmem: $(TARGET).bin
	od -An -tx4 -v -w4 $< | sed 's/^ *//' > $@
	@echo "ROM image: $@ ($$(wc -l < $@) words, $$(wc -c < $(TARGET).bin) bytes)"

clean:
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).vmem $(TARGET).dis $(TARGET).map

.PHONY: all clean
