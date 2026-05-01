.global swtch
.align 4

# swtch(from, to)  :  context switch between two tasks (or task <-> scheduler).
#
# Arguments (RISC-V integer calling convention):
#   a0 = struct context *from  : save current registers HERE
#   a1 = struct context *to    : load next registers FROM HERE
#
# After swtch returns, execution continues in the 'to' context as if
# swtch() had been called there : because that is exactly where *to's
# ra points (into yield() on a resume, or the task entry function on
# the very first run).
#
# We only save/restore callee-saved registers: ra, sp, s0-s11.
#
# Why not all registers?
# The RISC-V calling convention splits registers into two groups:
#   Caller-saved  (a0-a7, t0-t6): the CALLER saves these before any call.
#   Callee-saved  (ra, sp, s0-s11): the CALLEE must leave these intact.
#
# By the time a task calls yield() -> swtch(), it has already dealt with
# any caller-saved values it cared about. Only callee-saved registers need
# to survive the switch to the next task.
#
# Offsets in struct context (must match include/proc.h exactly):
#   ra:0   sp:8   s0:16  s1:24  s2:32  s3:40  s4:48  s5:56
#   s6:64  s7:72  s8:80  s9:88  s10:96 s11:104

swtch:
    # Save the current (from) context
    sd ra,    0(a0)
    sd sp,    8(a0)
    sd s0,   16(a0)
    sd s1,   24(a0)
    sd s2,   32(a0)
    sd s3,   40(a0)
    sd s4,   48(a0)
    sd s5,   56(a0)
    sd s6,   64(a0)
    sd s7,   72(a0)
    sd s8,   80(a0)
    sd s9,   88(a0)
    sd s10,  96(a0)
    sd s11, 104(a0)

    # Load the next (to) context
    ld ra,    0(a1)
    ld sp,    8(a1)
    ld s0,   16(a1)
    ld s1,   24(a1)
    ld s2,   32(a1)
    ld s3,   40(a1)
    ld s4,   48(a1)
    ld s5,   56(a1)
    ld s6,   64(a1)
    ld s7,   72(a1)
    ld s8,   80(a1)
    ld s9,   88(a1)
    ld s10,  96(a1)
    ld s11, 104(a1)

    # ret jumps to the newly loaded ra.
    # First run: ra == the task's entry function (set by task_init).
    # Resume:    ra == return address inside yield() after swtch().
    ret
