#include "defs.h"


int main() {
    uart_puts("Kira booting...\n");
    uart_puts("timer init");
    timerinit();
    uart_puts("kvm init");
    kvminit();
    uart_puts("kvm harts init");
    kvminithart();
    uart_puts("tasks init");
    task_init(0, a, "a");
    task_init(1, b, "b");
    uart_puts("scheduler init");
    scheduler();
    return 0;
}

void a(){
    while(1){
        uart_puts("a");
        yield();
    }
}

void b(){
    while(1){
        uart_puts("b");
        yield();
    }
}