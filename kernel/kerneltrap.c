#include "defs.h"
#include "types.h"

#define MTIME 0x200bff8
#define MTIMECMP 0x2004000

static uint64 counter = 0;

void kerneltrap() {
    uint64 mcause;
    __asm__ volatile("csrr %0, mcause" : "=r" (mcause));

    if(mcause & (1UL << 63)){
        // interrupt
        uart_puts("interrupt");

        uint64 cause_code = mcause & ~(1UL << 63);
        if (cause_code == 7){
            uart_puts("timer interrupt");
            uart_putx(counter++);
        }
        else{
            uart_puts("unknown interrupt");
        }
    }
    else{
        // exception
        uart_puts("exception");
        uart_putx(mcause);
    }

    //reset timer
    uint64 *mtime = (uint64*)MTIME;
    uint64 *mtimecmp = (uint64*)MTIMECMP;
    *mtimecmp = *mtime + 10000000;
}

void timerinit(){
    // schedule first timer interrupt
    uint64 *mtime = (uint64*)MTIME;
    uint64 *mtimecmp = (uint64*)MTIMECMP;

    *mtimecmp = *mtime + 1000000; // 1sec

    // timer enable
    __asm__ volatile("csrw mie, %0" : : "r" (1UL << 7));

    // global enable
    __asm__ volatile("csrw mstatus, %0" : : "r" (1UL << 3));

}
