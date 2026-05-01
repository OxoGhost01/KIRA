.global trap_vector
# mtvec in Direct mode requires a 4-byte-aligned address.
.align 4

# trap_vector : low-level entry point for ALL traps.
#
# When a trap fires (timer interrupt, exception, ...), the CPU:
#   1. Saves the interrupted PC into mepc.
#   2. Sets mcause to describe what happened.
#   3. Jumps to mtvec (us).
#
# Our job: save every register, call the C handler, restore, mret.
#
# Why save EVERYTHING?
# kerneltrap() is a regular C function. The compiler can use any
# caller-saved register it likes. If we don't preserve the interrupted
# code's full register state first, we'll corrupt it when we return.
#
# x0 is hardwired to zero and cannot be written : skip it.
# x2 (sp) is used as our own frame pointer here. We adjust it with
# 'addi sp, sp, -256' and undo that at the end : no separate save needed.

trap_vector:
    # Make room on the stack for registers x1-x31 (31 regs × 8 bytes = 248).
    # We allocate 256 bytes to keep the stack 16-byte aligned.
    addi sp, sp, -256

    # Save every register except x0 (always zero) and x2 (sp, managed above).
    sd x1,   0(sp)
    # sd x2, 8(sp) : skipped, managed via addi above and under
    sd x3,  16(sp)
    sd x4,  24(sp)
    sd x5,  32(sp)
    sd x6,  40(sp)
    sd x7,  48(sp)
    sd x8,  56(sp)
    sd x9,  64(sp)
    sd x10, 72(sp)
    sd x11, 80(sp)
    sd x12, 88(sp)
    sd x13, 96(sp)
    sd x14,104(sp)
    sd x15,112(sp)
    sd x16,120(sp)
    sd x17,128(sp)
    sd x18,136(sp)
    sd x19,144(sp)
    sd x20,152(sp)
    sd x21,160(sp)
    sd x22,168(sp)
    sd x23,176(sp)
    sd x24,184(sp)
    sd x25,192(sp)
    sd x26,200(sp)
    sd x27,208(sp)
    sd x28,216(sp)
    sd x29,224(sp)
    sd x30,232(sp)
    sd x31,240(sp)

    # Call the C handler. It reads mcause to decide what to do.
    call kerneltrap

    # Restore all saved registers in the same order.
    ld x1,   0(sp)
    ld x3,  16(sp)
    ld x4,  24(sp)
    ld x5,  32(sp)
    ld x6,  40(sp)
    ld x7,  48(sp)
    ld x8,  56(sp)
    ld x9,  64(sp)
    ld x10, 72(sp)
    ld x11, 80(sp)
    ld x12, 88(sp)
    ld x13, 96(sp)
    ld x14,104(sp)
    ld x15,112(sp)
    ld x16,120(sp)
    ld x17,128(sp)
    ld x18,136(sp)
    ld x19,144(sp)
    ld x20,152(sp)
    ld x21,160(sp)
    ld x22,168(sp)
    ld x23,176(sp)
    ld x24,184(sp)
    ld x25,192(sp)
    ld x26,200(sp)
    ld x27,208(sp)
    ld x28,216(sp)
    ld x29,224(sp)
    ld x30,232(sp)
    ld x31,240(sp)

    # Undo the stack allocation.
    addi sp, sp, 256

    # mret = Machine-mode Return.
    # Restores PC from mepc (set by hardware when the trap fired) and
    # returns to the interrupted privilege level.
    mret
