#include "defs.h"

/*
 * CLINT : Core Local Interruptor.
 *
 * The CLINT is the hardware block that drives machine-mode timer interrupts
 * on RISC-V. It exposes two MMIO registers:
 *
 *   mtime     (0x200bff8) : a 64-bit counter that ticks every cycle.
 *   mtimecmp  (0x2004000) : when mtime >= mtimecmp, a timer interrupt fires.
 *
 * To schedule the next interrupt we just write a new value into mtimecmp.
 * There is no "acknowledge" register to clear the interrupt; moving
 * mtimecmp into the future is enough.
 */
#define CLINT_MTIME    0x200bff8
#define CLINT_MTIMECMP 0x2004000
#define TIMER_INTERVAL 10000000   /* ~1 s at a 10 MHz simulated clock */

/* kerneltrap : C-level handler for all machine-mode traps.
 *
 * Called from the assembly stub in trap.S after all registers are saved.
 *
 * In RISC-V, a single vector (mtvec) catches everything: timer interrupts,
 * external interrupts, and synchronous exceptions (illegal instructions,
 * bad memory accesses, ...). mcause tells us which one we got:
 *
 *   bit 63 = 1  →  asynchronous interrupt
 *   bit 63 = 0  →  synchronous exception
 *   bits 62:0   →  cause code
 *
 * The only interrupt we actually handle is the machine timer (cause code 7).
 * Everything else is unexpected at this stage, so we panic.
 */
void kerneltrap(void) {
    uint64 mcause;
    __asm__ volatile("csrr %0, mcause" : "=r" (mcause));

    if (mcause & (1UL << 63)) {
        /* Interrupt */
        uint64 code = mcause & ~(1UL << 63);

        if (code == 7) {
            /* Machine timer interrupt : push mtimecmp forward for the next tick.
             * We read mtime again here instead of adding to the old mtimecmp
             * so we can never fall behind if the handler runs late. */
            uint64 *mtime    = (uint64 *)CLINT_MTIME;
            uint64 *mtimecmp = (uint64 *)CLINT_MTIMECMP;
            *mtimecmp = *mtime + TIMER_INTERVAL;
            return;
        }

        panic("PANIC: unexpected interrupt");
    } else {
        /* Synchronous exception
         * Illegal instruction, page fault, misaligned access, etc.
         * We have no recovery path : halt the machine. */
        panic("PANIC: unhandled exception");
    }
}

/* timerinit : arm the CLINT and enable machine-timer interrupts.
 *
 * Three things must happen before a timer interrupt can fire:
 *   1. Write a deadline into mtimecmp (mtime + N cycles from now).
 *   2. Set bit 7 of mie  (MTIE : Machine Timer Interrupt Enable).
 *   3. Set bit 3 of mstatus (MIE : global Machine Interrupt Enable).
 *
 * Without step 3, individual interrupt enables in mie have no effect.
 * Without step 2, the timer fires but the CPU ignores it.
 */
void timerinit(void) {
    uint64 *mtime    = (uint64 *)CLINT_MTIME;
    uint64 *mtimecmp = (uint64 *)CLINT_MTIMECMP;

    /* Schedule the first interrupt ~1 M cycles from now. */
    *mtimecmp = *mtime + 1000000;

    /* Enable the machine timer interrupt source (mie bit 7 = MTIE). */
    __asm__ volatile("csrw mie, %0" : : "r" (1UL << 7));

    /* Enable global machine interrupts (mstatus bit 3 = MIE). */
    __asm__ volatile("csrw mstatus, %0" : : "r" (1UL << 3));
}
