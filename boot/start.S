.section .text
.global _start

_start:
    # setup stack
    # setup trap vector
    # write to mtvec
    # enable machine interrupts
    # call kernel main (_start in hello.c)
    # Infinite loop if kernel returns
    la sp, stack_top
    la t0, trap_vector
    csrw mtvec, t0
    li t0, (1 << 3)
    csrw mstatus, t0
    call main
loop:
    wfi
    j loop