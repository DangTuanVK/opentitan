// Minimal boot test ROM with UART init
// Boot vector at 0x80, UART TX "OK\n"

#define UART_BASE    0x40000000
#define UART_CTRL    0x10        // TX_EN=bit0, NCO=bits[31:16]
#define UART_STATUS  0x14
#define UART_WDATA   0x1C        // TX FIFO write
#define GPIO_BASE    0x40010000
#define GPIO_OUT     0x04

// NCO for 115200 baud @ 24MHz: NCO = 115200 * 2^16 / 24000000 = 314 = 0x13A
// NCO for 115200 baud @ 100MHz: NCO = 115200 * 2^16 / 100000000 = 75 = 0x4B
// Shift left 16 for CTRL register format
#define UART_NCO_100M  0x004B0000
#define UART_TX_EN     0x1

.section .text
.global _start

// NOP sled from 0x00 to 0x7C (reset vector at 0x80)
.org 0x00
    nop
.org 0x80

_start:
    // 1. Init UART: set NCO + TX enable
    li   t0, UART_BASE
    li   t1, (UART_NCO_100M | UART_TX_EN)
    sw   t1, UART_CTRL(t0)

    // 2. Write 'O' to UART TX FIFO
    li   t1, 'O'
    sw   t1, UART_WDATA(t0)

    // 3. Write 'K' to UART TX FIFO
    li   t1, 'K'
    sw   t1, UART_WDATA(t0)

    // 4. Write '\n' to UART TX FIFO
    li   t1, '\n'
    sw   t1, UART_WDATA(t0)

    // 5. Write 0xAA to GPIO (visible in debug)
    li   t0, GPIO_BASE
    li   t1, 0xAA
    sw   t1, GPIO_OUT(t0)

    // 6. Infinite loop
    li   t2, 0
loop:
    addi t2, t2, 1
    j    loop
