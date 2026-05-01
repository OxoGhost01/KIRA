#ifndef DEFS_H
#define DEFS_H

/* Pull in the basic types and task/context structs used across the kernel. */
#include "types.h"
#include "proc.h"

/* uart.c */
void uart_putc(char c);                   /* send one byte to the serial port  */
void uart_puts(const char *s);            /* send a string, then a newline     */
void uart_putx(uint64 x);                 /* send a uint64 as hex + newline    */

/* panic.c */
void panic(const char *msg);              /* print msg and halt forever        */

/* kerneltrap.c */
void kerneltrap(void);                    /* C trap handler (called by trap.S) */
void timerinit(void);                     /* arm the CLINT timer               */

/* kalloc.c */
uint64 *kalloc(void);                     /* allocate one 4 KB page            */

/* vm.c */
pte  *walk(pagetable pt, uint64 va);
void  mappages(pagetable pt, uint64 va, uint64 pa, uint64 size, uint64 flags);
void  kvminit(void);                      /* build kernel page table           */
void  kvminithart(void);                  /* load page table into satp         */

/* swtch.S */
void swtch(struct context *from, struct context *to);   /* context switch     */

/* proc.c */
void task_init(int id, void (*fn)(void), char *name);
void scheduler(void);
void yield(void);

/* entry.c */
int  main(void);
void a(void);
void b(void);

#endif
