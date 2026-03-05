# Tools
CC = riscv64-unknown-elf-gcc
LD = riscv64-unknown-elf-ld
OBJCOPY = riscv64-unknown-elf-objcopy

# Flags
CFLAGS = -march=rv64g -mabi=lp64 -nostdlib -ffreestanding -O2 -Iinclude -mcmodel=medany
LDFLAGS = -T linker.ld

# Files
OBJS = boot/start.o kernel/hello.o kernel/uart.o

# Default target (first one = called by just `make`)
kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o kernel.elf $(OBJS)

# Pattern rules
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run in QEMU
run: kernel.elf
	qemu-system-riscv64 -machine virt -nographic -bios none -kernel kernel.elf

# Cleanup
clean:
	rm -f *.o boot/*.o kernel/*.o kernel.elf