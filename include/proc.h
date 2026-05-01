#ifndef PROC_H
#define PROC_H

#include "types.h"

/* context : the CPU registers that swtch() saves and restores.
 *
 * Only callee-saved registers live here. The RISC-V calling convention
 * says a function must preserve ra, sp, and s0-s11 across calls.
 * Caller-saved registers (a0-a7, t0-t6) are the caller's responsibility :
 * by the time swtch() is invoked from yield(), the task has already
 * handled any caller-saved values it cared about.
 *
 * The field order and byte offsets here MUST match the sd/ld instructions
 * in swtch.S. If you add a field, update both files.
 */
struct context {
    uint64 ra;     /*   0  return address */
    uint64 sp;     /*   8  stack pointer */
    uint64 s0;     /*  16  callee-saved general registers */
    uint64 s1;     /*  24  */
    uint64 s2;     /*  32  */
    uint64 s3;     /*  40  */
    uint64 s4;     /*  48  */
    uint64 s5;     /*  56  */
    uint64 s6;     /*  64  */
    uint64 s7;     /*  72  */
    uint64 s8;     /*  80  */
    uint64 s9;     /*  88  */
    uint64 s10;    /*  96  */
    uint64 s11;    /* 104  */
};

/* taskstate : lifecycle of a slot in the task table. */
enum taskstate {
    UNUSED   = 0,   /* slot is empty, never initialised  */
    RUNNABLE = 1,   /* ready to run, waiting for the CPU */
    RUNNING  = 2,   /* currently executing on the CPU    */
};

/* task : one cooperative task.
 *
 * When a task is not on the CPU, its register state is frozen in
 * 'context'. When the scheduler picks it, swtch() thaws that state
 * and execution resumes exactly where it left off inside yield().
 */
struct task {
    enum taskstate  state;    /* current lifecycle state                    */
    struct context  context;  /* saved register state (read/written by swtch.S) */
    uint64          stack;    /* base (low) address of the stack page       */
    char           *name;     /* human-readable label, for debugging        */
};

#endif
