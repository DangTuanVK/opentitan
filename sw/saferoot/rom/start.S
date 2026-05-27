// SafeRoot Boot ROM — RISC-V startup assembly
// Entry point at 0x00000000 (reset vector)
// Init: pinmux, clkmgr, stack, BSS, then jump to main

.section .vectors, "ax"
.global _start
.type _start, @function

// Register base addresses
.equ PINMUX_BASE,    0x40360000
.equ CLKMGR_BASE,    0x40340000
.equ RSTMGR_BASE,    0x40350000

// Pinmux MIO_OUTSEL registers (route peripherals to pads)
.equ PINMUX_MIO_OUTSEL_0, 0x15c   // MIO pad 0 output select
.equ PINMUX_MIO_OUTSEL_2, 0x164   // MIO pad 2 (GPIO_2 / UART TX)
.equ PINMUX_MIO_OUTSEL_3, 0x168   // MIO pad 3 (GPIO_3 / UART RX)

// MioOut enum + 3 = outsel value
// MioOutUart0Tx = 34, so outsel = 34 + 3 = 37
.equ OUTSEL_UART0_TX, 37
// MioOutGpioGpio0 = 0, outsel = 0 + 3 = 3
.equ OUTSEL_GPIO0,    3
.equ OUTSEL_GPIO1,    4
.equ OUTSEL_GPIO2,    5
.equ OUTSEL_GPIO3,    6

// Pinmux MIO_PERIPH_INSEL (route pads to peripheral inputs)
.equ PINMUX_MIO_PERIPH_INSEL_0, 0x03c  // first periph input select

// Clkmgr
.equ CLKMGR_CLK_ENABLES, 0x18

_start:
    // 1. Disable interrupts
    csrci   mstatus, 0x8

    // 2. Set global pointer
    .option push
    .option norelax
    la      gp, __global_pointer$
    .option pop

    // 3. Set stack pointer
    li      sp, 0x10020000

    // 4. Enable all peripheral clocks (clkmgr)
    //    CLK_ENABLES: bit 0=io_div4_peri, bit 1=io_div2_usb (unused)
    //    Default should be enabled, but write explicitly
    li      t0, CLKMGR_BASE
    li      t1, 0xF           // enable all clock groups
    sw      t1, CLKMGR_CLK_ENABLES(t0)

    // 5. Configure pinmux: route UART TX to MIO pad 2 (GPIO_2)
    li      t0, PINMUX_BASE

    // MIO_OUTSEL_2 = UART0_TX (37)
    li      t1, OUTSEL_UART0_TX
    sw      t1, PINMUX_MIO_OUTSEL_2(t0)

    // MIO_OUTSEL_0 = GPIO0 (3) — for test pass/fail LED
    li      t1, OUTSEL_GPIO0
    sw      t1, PINMUX_MIO_OUTSEL_0(t0)

    // MIO_OUTSEL_1 = GPIO1 (4)
    li      t1, OUTSEL_GPIO1
    sw      t1, (PINMUX_MIO_OUTSEL_0 + 4)(t0)

    // Small delay for clock/pinmux to settle
    li      t0, 100
1:  addi    t0, t0, -1
    bnez    t0, 1b

    // 6. Zero BSS section
    la      t0, _bss_start
    la      t1, _bss_end
2:  bge     t0, t1, 3f
    sw      zero, 0(t0)
    addi    t0, t0, 4
    j       2b
3:

    // 7. Jump to C main
    jal     ra, main

    // If main returns, loop forever
4:  wfi
    j       4b

.size _start, . - _start
