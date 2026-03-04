# Kira

A minimal RISC-V kernel built from scratch — bootloader, memory management, interrupts and scheduling, one step at a time.

![Status](https://img.shields.io/badge/status-in%20development-blue)
![Architecture](https://img.shields.io/badge/arch-RISC--V%2064-orange)
![License](https://img.shields.io/badge/license-MIT-green)

## Goals

Kira is a learning-driven kernel project. The objective is to understand how an OS works at the lowest level by implementing it from scratch, without relying on any existing runtime or standard library.

- No libc, no runtime, no magic
- Bare-metal RISC-V 64-bit (RV64GC)
- Runs on QEMU `virt` machine

## Roadmap

- [x] Toolchain setup & cross-compilation
- [x] Linker script — kernel placed at `0x80000000`
- [ ] Boot sequence — `_start` entry point, stack initialization
- [ ] UART driver — serial output
- [ ] Memory management — physical page allocator
- [ ] Interrupts & trap handling
- [ ] Scheduler

## Build

**Requirements**
- `riscv64-unknown-elf-gcc`
- `qemu-system-riscv64` (>= 7.2)

**Compile**
```bash
make
```

**Run**
```bash
make qemu
```

## Project Structure

```
kira/
├── kernel/
│   ├── entry.S       # _start, stack init
│   ├── main.c        # kernel entry point
│   └── linker.ld     # linker script
└── Makefile
```

## References

- [RISC-V ISA Specification](https://riscv.org/specifications/)
- [xv6 — MIT](https://github.com/mit-pdos/xv6-riscv) — used as reference
- [OSDev Wiki](https://wiki.osdev.org)

## License

MIT — see [LICENSE](LICENSE)