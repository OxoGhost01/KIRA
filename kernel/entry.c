#include "defs.h"

/* main : C entry point, called by boot/start.S after the stack is set up.
 *
 * This is the first C code that runs on the machine.
 * We are in M-mode (Machine mode) : the highest RISC-V privilege level.
 * No OS beneath us. No libc. No runtime. Just us and the hardware spec.
 */
int main(void) {
    uart_puts("Kira booting...");

    /* Arm the CLINT so we get periodic machine-timer interrupts.
     * The trap vector is already installed (start.S wrote it into mtvec),
     * so interrupts can fire as soon as timerinit() enables them. */
    timerinit();

    /* Build the Sv39 kernel page table and load it into satp.
     * We are in M-mode so the MMU does not translate our own accesses,
     * but the structure needs to exist for a future switch to S-mode. */
    kvminit();
    kvminithart();

    /* Create two cooperative tasks and hand off to the scheduler.
     * The scheduler loop never returns. */
    task_init(0, a, "a");
    task_init(1, b, "b");
    scheduler();

    return 0;
}

/* Task a : runs forever: print "a", then voluntarily give up the CPU. */
void a(void) {
    while (1) {
        uart_puts("a");
        yield();
    }
}

/* Task b : runs forever: print "b", then voluntarily give up the CPU. */
void b(void) {
    while (1) {
        uart_puts("b");
        yield();
    }
}
