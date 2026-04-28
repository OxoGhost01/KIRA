#include "defs.h"

#define PTE_V (1L << 0)
#define PTE_R (1L << 1)
#define PTE_W (1L << 2)
#define PTE_X (1L << 3)

#define PA2PTE(pa) (((uint64)(pa) >> 12) << 10)
#define PTE2PA(pte) (((pte) >> 10) << 12)

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