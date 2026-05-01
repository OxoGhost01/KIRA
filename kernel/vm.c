#include "defs.h"

/* PTE flag bits : packed into bits [3:0] of every 64-bit page table entry. */
#define PTE_V  (1L << 0)   /* Valid:   entry exists         */
#define PTE_R  (1L << 1)   /* Read                          */
#define PTE_W  (1L << 2)   /* Write                         */
#define PTE_X  (1L << 3)   /* Execute                       */

#define PAGE_SIZE  4096

/* Sv39 PTE / physical address conversion.
 *
 * A PTE stores the Physical Page Number in bits [53:10].
 * PA2PTE: shift the PA right by 12 (drop the page offset), then left by 10
 *         to land in the PPN field.
 * PTE2PA: the inverse : shift right 10, then left 12.
 */
#define PA2PTE(pa)     (((uint64)(pa) >> 12) << 10)
#define PTE2PA(pte)    (((pte) >> 10) << 12)
#define PGROUNDDOWN(a) ((a) & ~(uint64)(PAGE_SIZE - 1))

/* Physical addresses of the regions we identity-map in the kernel page table.
 * Identity mapping (VA == PA) keeps things simple: no need to translate. */
#define UART_BASE    0x10000000L
#define KERNEL_BASE  0x80000000L
#define STACK_BASE   0x80100000L

/* The one global kernel page table, allocated in kvminit(). */
pagetable ROOTPAGETABLE;

/* walk : traverse the Sv39 three-level page table tree to find the leaf PTE.
 *
 * Sv39 splits the 39-bit virtual address like this:
 *   VA[38:30]  →  index into the L2 (root) page table
 *   VA[29:21]  →  index into the L1 page table
 *   VA[20:12]  →  index into the L0 (leaf) page table
 *   VA[11: 0]  →  page offset (not used here)
 *
 * At each level we look up the entry. If it has PTE_V set, we follow the
 * physical address it points to. If not, we allocate a fresh page with
 * kalloc(), zero it, and install it as the next level.
 *
 * Returns a pointer to the leaf PTE slot so the caller can write the
 * final mapping into it.
 */
pte *walk(pagetable pt, uint64 va) {
    for (int level = 2; level > 0; level--) {
        /* Extract the 9-bit index for this level. */
        uint64 vpn   = (va >> (level * 9 + 12)) & 0x1FF;
        pte    entry = pt[vpn];

        if (entry & PTE_V) {
            /* Entry present : follow the pointer to the next level. */
            pt = (pagetable)PTE2PA(entry);
        } else {
            /* No table at this level yet : allocate and zero one. */
            pagetable new_pt = (pagetable)kalloc();
            for (int j = 0; j < 512; j++)
                new_pt[j] = 0;
            pt[vpn] = PA2PTE(new_pt) | PTE_V;
            pt = new_pt;
        }
    }

    /* Return the address of the leaf PTE for this VA. */
    return &pt[(va >> 12) & 0x1FF];
}

/* mappages : create identity mappings covering [va, va+size).
 *
 * For each page in the range, walk() locates (or creates) the leaf PTE
 * and we write the physical address into it with the requested flags.
 * We panic if a page is already mapped : double-mapping is always a bug.
 */
void mappages(pagetable pt, uint64 va, uint64 pa, uint64 size, uint64 flags) {
    pa          = PGROUNDDOWN(pa);
    va          = PGROUNDDOWN(va);
    uint64 va_end = va + size;

    for (; va < va_end; pa += PAGE_SIZE, va += PAGE_SIZE) {
        pte *leaf = walk(pt, va);
        if (*leaf & PTE_V)
            panic("mappages: remapping an already-mapped page");
        *leaf = PA2PTE(pa) | flags | PTE_V;
    }
}

/* kvminit : allocate and populate the kernel page table.
 *
 * We use identity mappings (VA == PA) for everything the kernel touches:
 *   UART      (R/W)  : so we can write to the serial port
 *   Kernel    (R/X)  : code and read-only data
 *   Stack     (R/W)  : the boot stack defined in linker.ld
 *
 * The MMU has no effect in M-mode (our privilege level), so this table
 * is mainly groundwork for a future switch to S-mode.
 */
void kvminit(void) {
    ROOTPAGETABLE = (pagetable)kalloc();
    for (int i = 0; i < 512; i++)
        ROOTPAGETABLE[i] = 0;

    mappages(ROOTPAGETABLE, UART_BASE,   UART_BASE,   0x1000,   PTE_R | PTE_W);
    mappages(ROOTPAGETABLE, KERNEL_BASE, KERNEL_BASE, 0x100000, PTE_R | PTE_X);
    mappages(ROOTPAGETABLE, STACK_BASE,  STACK_BASE,  0x1000,   PTE_R | PTE_W);
}

/* kvminithart : install the kernel page table and flush the TLB.
 *
 * satp (Supervisor Address Translation and Protection) controls the MMU.
 * Format: [mode(4)] [ASID(16)] [PPN(44)]
 *   mode 8 = Sv39 (3-level, 39-bit virtual addresses)
 *   PPN    = root page table address >> 12
 *
 * sfence.vma flushes any stale address translations cached in the TLB.
 * You MUST do this after changing satp, or the CPU might use old mappings.
 */
void kvminithart(void) {
    uint64 satp = (8L << 60) | ((uint64)ROOTPAGETABLE >> 12);
    __asm__ volatile("csrw satp, %0" : : "r" (satp));
    __asm__ volatile("sfence.vma");
}
