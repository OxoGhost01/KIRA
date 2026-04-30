#include "defs.h"


int main() {
    uart_puts("Kira booting...\n");
    timerinit();
    kvminit();
    kvminithart();
    while(1){
        __asm__ volatile("wfi");
    }
    return 0;
}