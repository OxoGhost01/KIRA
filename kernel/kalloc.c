#include "defs.h"

/* 'end' is a symbol exported by linker.ld : it marks the first byte after
 * the kernel's .bss section and boot stack. Everything past it is free RAM
 * we can hand out as pages. */
extern char end[];

/* next : bump pointer. Zero means "not yet initialised". */
static char *next = 0;

/* kalloc : allocate one 4 KB page.
 *
 * This is a bump allocator: the simplest allocator imaginable.
 * We keep a pointer to the next free page and just advance it.
 * There is no free() : once a page is given out it is gone forever.
 *
 * That is fine for a kernel this small. We allocate page tables and
 * task stacks at boot and never need to reclaim them.
 *
 * One gotcha I ran into: 'end' must come after ALL BSS sections in the
 * linker script (.bss AND .sbss). If any BSS variable ends up past 'end',
 * kalloc silently overwrites live kernel globals : and that is a very
 * confusing bug to debug.
 */
uint64 *kalloc(void) {
    if (next == 0)
        next = end;         /* lazy init: first call starts at kernel image end */

    uint64 *page = (uint64 *)next;
    next += 4096;           /* advance past the page we just handed out */
    return page;
}
