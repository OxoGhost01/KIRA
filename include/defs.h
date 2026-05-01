#ifndef DEFS_H
#define DEFS_H

#include "types.h"
#include "proc.h"

// uart.c
void uart_putc(char c);
void uart_puts(char *s);
void uart_putx(uint64 x);

// panic.c
void panic(char *s);

// kerneltrap.c
void kerneltrap();
void timerinit();

// kalloc.c
uint64 *kalloc();

// vm.c
pte *walk(pagetable pt, uint64 va);
void mappages(pagetable pt, uint64 va, uint64 pa, uint64 size, uint64 flags);
void kvminit();
void kvminithart();

// swtch.S
extern void swtch(struct context *, struct context *);

// proc.c
void task_init(int id, void (*fn)(void), char *name);
void scheduler();
void yield();

// entry.c
int main();
void a();
void b();

#endif