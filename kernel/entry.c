#include "defs.h"


int main() {
    uart_puts("Kira booting...\n");
    timerinit();
    while(1){
        __asm__ volatile("wfi");
    }
    return 0;
}