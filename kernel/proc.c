#include "defs.h"

#define MAX_TASKS  16

/* Global task table. Everything is statically allocated : no dynamic
 * task creation. Slots that were never initialised have state == UNUSED. */
struct task tasks[MAX_TASKS];
int current_task = 0;

/* The scheduler's own saved CPU context.
 *
 * When swtch() switches FROM a task TO the scheduler, it parks the
 * scheduler's ra and sp here so it can resume exactly where it left off
 * (right after the swtch() call inside scheduler()). */
struct context scheduler_context;

/* task_init : fill in one task slot so the scheduler can run it.
 *
 * We allocate a fresh 4 KB page for the stack, set sp to the top of it
 * (stacks grow downward on RISC-V), and point ra at the task function.
 *
 * On the very first swtch() into this task, swtch loads ra and sp from
 * this context and executes 'ret', which jumps directly into fn() with
 * a clean, empty stack. No special first-run trampoline needed.
 */
void task_init(int id, void (*fn)(void), char *name) {
    tasks[id].state      = RUNNABLE;
    tasks[id].name       = name;
    tasks[id].stack      = (uint64)kalloc();
    tasks[id].context.sp = tasks[id].stack + 4096;  /* top of stack page */
    tasks[id].context.ra = (uint64)fn;              /* task entry point  */
}

/* scheduler : cooperative round-robin scheduler, runs forever.
 *
 * Scans the task table in a tight loop. When it finds a RUNNABLE task it
 * calls swtch() to switch into it. The task runs until it calls yield(),
 * which switches back here. We then mark the task RUNNABLE again and move
 * on to the next slot.
 *
 * scheduler_context is this loop's "saved seat". It holds the stack pointer
 * and return address that bring us back to the line after swtch() each time
 * a task yields.
 */
void scheduler(void) {
    while (1) {
        for (int i = 0; i < MAX_TASKS; i++) {
            if (tasks[i].state == RUNNABLE) {
                current_task   = i;
                tasks[i].state = RUNNING;
                swtch(&scheduler_context, &tasks[i].context);
                /* --- task ran and called yield(), we resume here --- */
                tasks[i].state = RUNNABLE;
            }
        }
    }
}

/* yield : voluntarily give up the CPU back to the scheduler.
 *
 * Saves the calling task's callee-saved registers into its context struct
 * and restores the scheduler's registers. Execution continues in
 * scheduler() right after the swtch() call that last switched into us.
 */
void yield(void) {
    swtch(&tasks[current_task].context, &scheduler_context);
}
