/* linker.ld : physical memory layout for the KIRA kernel.
 *
 * QEMU's 'virt' machine starts DRAM at 0x80000000, so we place everything
 * there. The CPU resets to 0x80000000, which must be the first instruction
 * of _start (boot/start.S).
 *
 * Critical ordering rule:
 *   'end' must come AFTER every BSS section (.bss AND .sbss).
 *   kalloc() uses 'end' as its first free address. If any global variable
 *   lives past 'end', kalloc silently overwrites it : a very confusing bug
 *   that manifests as random corruption of kernel data structures.
 *
 * RISC-V note:
 *   The compiler puts small globals in .sbss (small BSS, accessed via the
 *   global pointer gp). The linker places .sbss as an orphan section if
 *   you forget to list it explicitly : usually after 'end', breaking kalloc.
 *   We list it explicitly below to keep it in front of 'end'.
 */

ENTRY(_start)

SECTIONS {
    /* Code and read-only data, starting at DRAM base */
    . = 0x80000000;

    .text : {
        *(.text)
        *(.text.*)
    }

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    /* Initialised globals */
    .data : {
        *(.data)
        *(.data.*)
    }

    /* Zero-initialised globals (BSS)
     *
     * Both .bss and .sbss must be listed here so they land before 'end'.
     * COMMON covers tentative definitions (uninitialized globals without
     * an explicit size, uncommon in kernel code but handled for safety). */
    .bss : {
        *(.bss)
        *(.bss.*)
        *(.sbss)
        *(.sbss.*)
        *(COMMON)
    }

    /* Boot stack (4 KB) 
     *
     * Aligned to a page boundary so kalloc() can start cleanly on the
     * next page after it.
     * stack_top is loaded into sp in boot/start.S.
     * end      is where kalloc() begins handing out pages. */
    . = ALIGN(0x1000);
    stack_bottom = .;
    . += 0x1000;
    stack_top = .;
    end = .;
}
