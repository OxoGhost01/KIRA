#ifndef DEFS_H
#define DEFS_H

#include "types.h"

// uart.c
void uart_putc(char c);
void uart_puts(char *s);
void uart_putx(uint64 x);

// kerneltrap.c
void kerneltrap();
void timerinit();

// kalloc.c
uint64 *kalloc();

// vm.c
pte *walk(pagetable pt, uint64 va);

#endif