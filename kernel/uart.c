#include "defs.h"

/*
 * UART 16550 driver : the only way we can talk to the outside world.
 *
 * The QEMU 'virt' machine puts a 16550-compatible UART at 0x10000000.
 * It is a memory-mapped device: reading and writing specific addresses
 * talks directly to the hardware registers. No I/O port instructions
 * needed : just ordinary loads and stores.
 *
 * We only need two registers for basic output:
 *   THR  (offset 0) : Transmit Holding Register: write a byte here to send it
 *   LSR  (offset 5) : Line Status Register:     bit 5 = transmit buffer empty
 *
 * The 'volatile' keyword on the macros is critical. Without it the compiler
 * is allowed to cache MMIO reads, which would break the busy-wait loop :
 * it would read LSR once, cache it, and loop forever even after the hardware
 * clears the busy bit.
 */
#define UART_BASE      0x10000000L
#define UART_THR       (*(volatile char *)(UART_BASE + 0))
#define UART_LSR       (*(volatile char *)(UART_BASE + 5))
#define UART_LSR_EMPTY (1 << 5)

/* uart_putc : write one character to the serial port.
 *
 * Spin-wait until the transmit buffer is empty, then drop the byte in.
 * On QEMU this busy-wait is essentially instant; on real hardware it
 * depends on the baud rate.
 */
void uart_putc(char c) {
    while (!(UART_LSR & UART_LSR_EMPTY));
    UART_THR = c;
}

/* uart_puts : write a NUL-terminated string followed by a newline. */
void uart_puts(const char *s) {
    while (*s)
        uart_putc(*s++);
    uart_putc('\n');
}

/* uart_putx : write a uint64 as lowercase hex followed by a newline.
 *
 * Useful for dumping addresses or CSR values when debugging.
 * We build the digits in reverse into a local buffer, then print them
 * forwards : the classic "itoa" trick without needing stdlib.
 */
void uart_putx(uint64 x) {
    char buf[16];
    int  i = 0;

    if (x == 0) {
        uart_putc('0');
        uart_putc('\n');
        return;
    }

    while (x) {
        buf[i++] = "0123456789abcdef"[x & 0xf];
        x >>= 4;
    }

    /* digits are stored lowest-first in buf, print them highest-first */
    while (i-- > 0)
        uart_putc(buf[i]);

    uart_putc('\n');
}
