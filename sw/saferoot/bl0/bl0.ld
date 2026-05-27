/* SafeRoot BL0 (ROM_EXT) Linker Script */
/* BL0 runs from Flash at 0x20000000 (32KB), RAM at 0x10000000 (128KB) */

ENTRY(_bl0_start)

MEMORY {
    flash (rx)  : ORIGIN = 0x20000000, LENGTH = 0x8000   /* 32 KB */
    ram   (rwx) : ORIGIN = 0x10000000, LENGTH = 0x20000  /* 128 KB */
}

SECTIONS {
    /* BL0 manifest / entry stub at flash base */
    .bl0_header : {
        KEEP(*(.bl0_header))
    } > flash

    /* Code */
    .text : {
        *(.text .text.*)
    } > flash

    /* Read-only data */
    .rodata : {
        *(.rodata .rodata.*)
        *(.srodata .srodata.*)
    } > flash

    /* Provide end-of-ROM symbol for data copy */
    _data_load = LOADADDR(.data);

    /* Global pointer — middle of RAM for efficient access via gp-relative */
    __global_pointer$ = 0x10010000;

    /* Initialized data (stored in flash, copied to RAM at startup) */
    .data : AT(LOADADDR(.rodata) + SIZEOF(.rodata)) {
        _data_start = .;
        *(.data .data.*)
        *(.sdata .sdata.*)
        _data_end = .;
    } > ram

    /* BSS — zero-initialized data in RAM */
    .bss (NOLOAD) : {
        _bss_start = .;
        *(.bss .bss.*)
        *(.sbss .sbss.*)
        *(COMMON)
        _bss_end = .;
    } > ram

    /* Stack at top of RAM (grows downward) */
    _stack_top = ORIGIN(ram) + LENGTH(ram);
}
