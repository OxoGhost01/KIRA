#include "defs.h"
#include "types.h"

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
    uart_putc('\n');
}

void uart_putx(uint64 x){
    char buf[16];
    int i = 0;
    while(x){
        buf[i++] = "0123456789abcdef"[x & 0xf];
        x >>= 4;
    }
    while(i-- > 0){
        uart_putc(buf[i]);
    }
    uart_putc('\n');
}
