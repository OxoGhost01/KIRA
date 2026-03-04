.section .text
.global _start

_start:
    # setup stack
    # call kernel main (_start in hello.c)
    # Infinite loop if kernel returns
    la sp, stack_top
    call main
loop:
    j loop