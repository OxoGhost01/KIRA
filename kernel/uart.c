#include "defs.h"

#define UART_BASE 0x10000000L
#define UART_THR (*(volatile char*)(UART_BASE + 0))
#define UART_LSR (*(volatile char*)(UART_BASE + 5))
#define UART_LSR_EMPTY (1 << 5)

void uart_putc(char c) {
    while (!(UART_LSR & UART_LSR_EMPTY));
    UART_THR = c;
}

void uart_puts(char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}