#include "defs.h"

#define PTE_V (1L << 0)
#define PTE_R (1L << 1)
#define PTE_W (1L << 2)
#define PTE_X (1L << 3)

#define PAGE_SIZE 4096

#define PA2PTE(pa) (((uint64)(pa) >> 12) << 10)
#define PTE2PA(pte) (((pte) >> 10) << 12)
#define PGROUNDDOWN(a) ((a) & ~(0xFFF))

#define UART_BASE 0x10000000L
#define KERNEL_BASE 0x80000000L
#define STACK_BASE 0x80100000L
pagetable ROOTPAGETABLE;

pte *walk(pagetable pt, uint64 va){
    for(int i = 2; i > 0; i--){
        uint64 vpn = (va >> (i * 9 + 12)) & 0x1FF;
        pte ptentry = pt[vpn];
        if(ptentry & PTE_V){
            pt = (pagetable)PTE2PA(ptentry);
        }
        else{
            pagetable pt_new = (pagetable)kalloc();
            for(int j = 0; j < 512; j++)
                pt_new[j] = 0;
            pt[vpn] = PA2PTE(pt_new) | PTE_V;
            pt = pt_new;
        }
    }
    return &pt[(va >> 12) & 0x1FF];
}

void mappages(pagetable pt, uint64 va, uint64 pa, uint64 size, uint64 flags){
    pa = PGROUNDDOWN(pa);
    va = PGROUNDDOWN(va);
    pte *leaf;
    uint64 end = va + size;
    for(;va < end; pa += PAGE_SIZE, va += PAGE_SIZE){
        leaf = walk(pt, va);
        if(*leaf & PTE_V){
            panic("mappages: remap"); 
        }
        else{
            *leaf = PA2PTE(pa) | flags | PTE_V;
        }
    }
}

void kvminit(){
    ROOTPAGETABLE = kalloc();
    for(int i = 0; i < 512; i++){
        ROOTPAGETABLE[i] = 0;
    }
    mappages(ROOTPAGETABLE, UART_BASE, UART_BASE, 0x1000, PTE_R | PTE_W);
    mappages(ROOTPAGETABLE, KERNEL_BASE, KERNEL_BASE, 0x100000, PTE_R | PTE_X);
    mappages(ROOTPAGETABLE, STACK_BASE, STACK_BASE, 0x1000, PTE_R | PTE_W);
}

void kvminithart(){
    uint64 satp = (8L << 60) | ((uint64)ROOTPAGETABLE >> 12);
    __asm__ volatile("csrw satp, %0" : : "r" (satp));
    __asm__ volatile("sfence.vma");
}