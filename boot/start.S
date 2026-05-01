.section .text
.global _start

# _start : the very first instruction that runs when QEMU loads our kernel.
#
# QEMU's 'virt' machine resets the PC to 0x80000000. The linker places
# _start there (see linker.ld). At this point:
#   - We are in M-mode (Machine mode), the highest RISC-V privilege level.
#   - The stack pointer is UNDEFINED. We must set it up before any C call.
#   - All interrupts are disabled.
#   - Nothing else is initialised.
#
# We do the bare minimum here before handing off to C: set up the stack,
# install the trap vector, enable interrupts globally, then call main().

_start:
    #! Stack pointer setup
    # 'stack_top' is a symbol defined in linker.ld pointing to the high end
    # of a 4 KB region we reserved just for the boot stack.
    # RISC-V stacks grow downward, so we start sp at the TOP of the region.
    la sp, stack_top

    #! Trap vector installation
    # mtvec is the Machine Trap Vector CSR. When ANY trap fires (timer
    # interrupt, external interrupt, or synchronous exception), the CPU
    # automatically jumps to the address stored in mtvec.
    #
    # We're using Direct mode: bits [1:0] = 00, which means ALL traps go
    # to the exact address in mtvec. (The alternative, Vectored mode,
    # would add 4*cause_code to the base : more complex, not needed yet.)
    la   t0, trap_vector
    csrw mtvec, t0

    #! Global interrupt enable 
    # mstatus bit 3 = MIE (Machine Interrupt Enable).
    # Without this, no interrupt ever reaches the CPU even if individually
    # enabled in mie. Think of it as the master switch.
    li   t0, (1 << 3)
    csrw mstatus, t0

    #! Jump to C
    # 'call' is a pseudo-instruction that expands to auipc + jalr.
    # It saves the return address in ra and jumps to main().
    call main

# If main() ever returns (it shouldn't : scheduler() loops forever),
# spin here with wfi (Wait For Interrupt).
# wfi suspends the CPU until an interrupt arrives, keeping power usage low.
loop:
    wfi
    j loop
