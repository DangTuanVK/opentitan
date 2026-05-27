/* SafeRoot Boot ROM Linker Script */
/* ROM at 0x00000000 (32KB), RAM at 0x10000000 (128KB) */

ENTRY(_start)

MEMORY {
    rom (rx)  : ORIGIN = 0x00000000, LENGTH = 0x8000   /* 32 KB */
    ram (rwx) : ORIGIN = 0x10000000, LENGTH = 0x20000  /* 128 KB */
}

SECTIONS {
    /* Vector table + startup code at ROM base */
    .vectors : {
        *(.vectors)
    } > rom

    /* Code */
    .text : {
        *(.text .text.*)
    } > rom

    /* Read-only data */
    .rodata : {
        *(.rodata .rodata.*)
        *(.srodata .srodata.*)
    } > rom

    /* Global pointer — middle of RAM for efficient access */
    __global_pointer$ = 0x10010000;

    /* Initialized data (copy from ROM to RAM) */
    .data : AT(LOADADDR(.rodata) + SIZEOF(.rodata)) {
        _data_start = .;
        *(.data .data.*)
        *(.sdata .sdata.*)
        _data_end = .;
    } > ram

    /* BSS — zero-initialized data */
    .bss (NOLOAD) : {
        _bss_start = .;
        *(.bss .bss.*)
        *(.sbss .sbss.*)
        *(COMMON)
        _bss_end = .;
    } > ram

    /* Stack at top of RAM */
    _stack_top = ORIGIN(ram) + LENGTH(ram);
}
