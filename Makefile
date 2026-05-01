# Tools
CC      = riscv64-unknown-elf-gcc
LD      = riscv64-unknown-elf-ld
OBJCOPY = riscv64-unknown-elf-objcopy

# Compiler flags
#   -march=rv64g     : RISC-V 64-bit, G = IMAFD standard extension set
#   -mabi=lp64       : LP64 ABI (long and pointer = 64 bits)
#   -nostdlib        : no standard library (we are the OS)
#   -ffreestanding   : no assumptions about a hosted C environment
#   -O2              : optimise (also needed for some size-critical code)
#   -mcmodel=medany  : PC-relative addressing, code/data anywhere in 64-bit space
#   -Wall -Wextra    : enable all common warnings so nothing hides in the noise
CFLAGS = \
    -march=rv64g      \
    -mabi=lp64        \
    -nostdlib         \
    -ffreestanding    \
    -O2               \
    -Iinclude         \
    -mcmodel=medany   \
    -Wall             \
    -Wextra

LDFLAGS = -T linker.ld

OBJS = \
    boot/start.o        \
    kernel/entry.o      \
    kernel/uart.o       \
    kernel/kerneltrap.o \
    kernel/trap.o       \
    kernel/kalloc.o     \
    kernel/vm.o         \
    kernel/panic.o      \
    kernel/proc.o       \
    kernel/swtch.o

# Default target: link everything into one ELF
kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o kernel.elf $(OBJS)

# Compile C sources
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble .S sources (same flags so the preprocessor works on them too)
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

# Run in QEMU  (press Ctrl-A then X to exit)
run: kernel.elf
	qemu-system-riscv64 -machine virt -nographic -bios none -kernel kernel.elf

# Remove all build artefacts
clean:
	rm -f *.o boot/*.o kernel/*.o kernel.elf
