# KIRA

A minimal RISC-V kernel written from scratch in C and RISC-V assembly.

This was my first time writing real assembly, first time touching hardware directly through memory-mapped I/O, and first time building anything that runs without an OS beneath it. No libc. No runtime. Just a CPU spec, a lot of GDB sessions, and eventually: two tasks printing `a` and `b` forever.

![Status](https://img.shields.io/badge/status-working-brightgreen)
![Architecture](https://img.shields.io/badge/arch-RISC--V%2064-orange)
![License](https://img.shields.io/badge/license-MIT-green)

---

## What it does

| Subsystem | Description |
|---|---|
| Bootloader | Custom RISC-V assembly (`_start`), sets up stack, installs trap vector |
| UART driver | 16550 MMIO driver : the only I/O we have |
| Trap handling | Single vector for all interrupts and exceptions (M-mode) |
| Timer | CLINT machine-timer interrupt, fires every ~1 s |
| Virtual memory | Sv39 three-level page tables, identity-mapped kernel |
| Allocator | Page-granularity bump allocator (simplest thing that works) |
| Scheduler | Cooperative round-robin over a static task table |
| Context switch | Hand-written assembly : swaps callee-saved registers only |

Expected output after `make run`:

```
Kira booting...
a
b
a
b
...
```

---

## Build & Run

### Prerequisites

```bash
sudo apt install gcc-riscv64-unknown-elf qemu-system-misc
```

### Build

```bash
make
```

### Run in QEMU

```bash
make run
```

Press `Ctrl-A` then `X` to exit QEMU.

### Debug with GDB

```bash
# Terminal 1 : start QEMU, frozen at first instruction
qemu-system-riscv64 -machine virt -nographic -bios none -kernel kernel.elf -s -S

# Terminal 2 : attach GDB
gdb-multiarch kernel.elf
(gdb) target remote :1234
(gdb) break main
(gdb) continue
```

---

## Project Structure

```
KIRA/
├── boot/
│   └── start.S          # _start: stack setup, mtvec install, call main()
├── kernel/
│   ├── entry.c          # main(), task a(), task b()
│   ├── uart.c           # 16550 UART driver (uart_putc / uart_puts / uart_putx)
│   ├── kalloc.c         # bump allocator : hands out 4 KB pages from 'end'
│   ├── vm.c             # Sv39 page tables: walk(), mappages(), kvminit()
│   ├── kerneltrap.c     # C-level trap handler + CLINT timer init
│   ├── trap.S           # assembly trap entry: save all regs, call kerneltrap
│   ├── swtch.S          # context switch: swap callee-saved registers
│   ├── proc.c           # task table, scheduler(), yield()
│   └── panic.c          # print message and halt
├── include/
│   ├── types.h          # uint8/16/32/64, pte, pagetable
│   ├── defs.h           # all function declarations
│   └── proc.h           # struct context, struct task, enum taskstate
├── linker.ld            # memory layout: .text / .bss / stack / kalloc heap
├── Makefile
└── ARCHITECTURE.md      # detailed technical walkthrough
```

---

## Roadmap

- [x] Toolchain & cross-compilation
- [x] Linker script : kernel at `0x80000000`
- [x] Boot sequence : `_start`, stack, mtvec
- [x] UART driver : serial output via MMIO
- [x] Trap vector : interrupts and exceptions
- [x] CLINT timer interrupt
- [x] Physical page allocator (bump)
- [x] Sv39 virtual memory (page tables)
- [x] Cooperative scheduler + context switching

---

## References

- [xv6-riscv : MIT PDOS](https://github.com/mit-pdos/xv6-riscv) : used as reference throughout
- [RISC-V Privileged Spec](https://github.com/riscv/riscv-isa-manual) : M-mode, CSRs, trap handling
- [OSDev Wiki : RISC-V](https://wiki.osdev.org/RISC-V)

## License

MIT
