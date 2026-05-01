# KIRA : Architecture

A walkthrough of every subsystem, roughly in the order things happen at runtime.

---

## Memory Layout

The linker script ([linker.ld](linker.ld)) places everything starting at `0x80000000`, which is where QEMU's `virt` machine puts DRAM. Sections are laid out like this:

```
0x80000000  ┌──────────────────────┐
            │  .text               │  kernel code (R-X)
            ├──────────────────────┤
            │  .rodata             │  string literals, constants (R)
            ├──────────────────────┤
            │  .data               │  initialised globals (RW)
            ├──────────────────────┤
            │  .bss + .sbss        │  zero-initialised globals (RW)
            ├──────────────────────┤  ← stack_bottom
            │  boot stack (4 KB)   │  grows downward
            ├──────────────────────┤  ← stack_top = end
            │  kalloc pages        │  dynamic: page tables, task stacks
            └──────────────────────┘
```

**Why does .sbss matter?**
The RISC-V compiler puts some small globals in `.sbss` (Small BSS), a section accessed via the global pointer register (`gp`). If you forget to list `.sbss` in the linker script, the linker places it as an orphan section *after* `end` : then `kalloc()` silently overwrites live kernel variables. This produced a very confusing `mcause=2` (illegal instruction) bug: `kalloc` was overwriting the page tables it was supposed to build, which in turn wrote PTE values into kernel code, corrupting the first instruction of task `a()`.

---

## Boot Sequence

```
QEMU reset
    └─> PC = 0x80000000
            └─> _start  (boot/start.S)
                    ├─ sp = stack_top          (set up the stack)
                    ├─ mtvec = trap_vector      (install trap handler)
                    ├─ mstatus.MIE = 1          (enable global interrupts)
                    └─> main()  (kernel/entry.c)
                            ├─ uart_puts("Kira booting...")
                            ├─ timerinit()
                            ├─ kvminit()
                            ├─ kvminithart()
                            ├─ task_init(0, a, "a")
                            ├─ task_init(1, b, "b")
                            └─> scheduler()   [never returns]
```

We run entirely in M-mode (Machine mode) : the highest RISC-V privilege level. There is no firmware, no SBI, nothing between us and the hardware.

---

## Trap Handling

### Setup

`start.S` writes the address of `trap_vector` into the `mtvec` CSR before enabling interrupts. We use Direct mode (`mtvec[1:0] = 00`), so every trap : timer, exception, anything : jumps to the same address.

### Flow

```
Trap fires
    └─> CPU saves PC into mepc, sets mcause
    └─> CPU jumps to mtvec = trap_vector  (kernel/trap.S)
            ├─ addi sp, sp, -256       (make room on stack)
            ├─ sd x1..x31, N(sp)       (save all registers)
            ├─ call kerneltrap()        (kernel/kerneltrap.c)
            │       ├─ read mcause
            │       ├─ if interrupt + cause=7: reset CLINT timer, return
            │       └─ else: panic()
            ├─ ld x1..x31, N(sp)       (restore all registers)
            ├─ addi sp, sp, 256
            └─ mret                    (return to mepc)
```

### mcause decoding

Bit 63 of `mcause` distinguishes interrupts from exceptions:
- `mcause[63] = 1` → asynchronous interrupt; `mcause[62:0]` = cause code
- `mcause[63] = 0` → synchronous exception (illegal instruction, fault, …)

We handle exactly one interrupt: machine timer (cause code 7). Everything else calls `panic()`.

---

## Timer (CLINT)

The CLINT (Core Local Interruptor) is the timer block built into RISC-V. Two MMIO registers control it:

| Register | Address | Description |
|---|---|---|
| `mtime` | `0x200bff8` | 64-bit counter, increments every cycle |
| `mtimecmp` | `0x2004000` | When `mtime >= mtimecmp`, a timer interrupt fires |

To schedule the next interrupt, write a new deadline: `*mtimecmp = *mtime + N`. There is no "acknowledge" register : advancing `mtimecmp` is all it takes.

Three bits must be set before any interrupt fires:
1. `mie[7]` = MTIE : enable the machine timer interrupt source
2. `mstatus[3]` = MIE : global machine interrupt enable (the master switch)
3. A valid `mtimecmp` deadline

---

## Virtual Memory (Sv39)

We build a kernel page table even though M-mode does not use it for our own code : it is groundwork for a future switch to S-mode.

### Sv39 address layout

A 39-bit virtual address is split into four fields:

```
 38        30 29       21 20       12 11          0
┌────────────┬────────────┬────────────┬────────────┐
│  VPN[2]    │  VPN[1]    │  VPN[0]    │   offset   │
│  (9 bits)  │  (9 bits)  │  (9 bits)  │  (12 bits) │
└────────────┴────────────┴────────────┴────────────┘
     L2 index     L1 index     L0 index
```

Each level is a 4 KB page of 512 eight-byte entries (PTEs). `walk()` descends the tree, allocating intermediate tables as needed, and returns a pointer to the leaf PTE.

### PTE format

```
 63      54 53           10 9    8  7  6  5  4  3  2  1  0
┌──────────┬───────────────┬──────┬──┬──┬──┬──┬──┬──┬──┬──┐
│ reserved │  PPN (44 bit) │ RSW  │D │A │G │U │X │W │R │V │
└──────────┴───────────────┴──────┴──┴──┴──┴──┴──┴──┴──┴──┘
```

We only use V (valid), R (read), W (write), X (execute).

### Kernel mappings (identity, VA == PA)

| Region | Physical address | Size | Flags |
|---|---|---|---|
| UART | `0x10000000` | 4 KB | R/W |
| Kernel code | `0x80000000` | 1 MB | R/X |
| Boot stack | `0x80100000` | 4 KB | R/W |

### Activating the MMU

`kvminithart()` writes the root page table's physical page number and mode 8 (Sv39) into `satp`, then executes `sfence.vma` to flush the TLB.

---

## Allocator (kalloc)

The simplest allocator imaginable: a bump pointer starting at `end` (the symbol marking the end of all static kernel data). Each call to `kalloc()` returns the next 4 KB-aligned page and advances the pointer. There is no `free()`.

```
end = 0x80003000
        │
        ▼
┌───────────────────────────────────────────────────┐
│  page 0  │  page 1  │  page 2  │  page 3  │  ...  │
│ (root PT)│ (L1 PT)  │(task0 stk│(task1 stk│       │
└───────────────────────────────────────────────────┘
  next →
```

---

## Scheduler & Context Switching

### Task structure

Each task has a `struct task` containing:
- `state` : UNUSED / RUNNABLE / RUNNING
- `context` : saved register state (ra, sp, s0-s11)
- `stack` : base address of the task's stack page

### First run

`task_init()` sets `context.ra = fn` and `context.sp = stack_top`. On the first `swtch()` into the task, `ret` jumps directly to `fn` with a clean stack. No trampoline needed.

### Cooperative switch

```
scheduler()                     task a()
    │                               │
    ├─ swtch(&sched_ctx, &a.ctx) ──>│  (task a starts / resumes)
    │                               │
    │                               ├─ uart_puts("a")
    │                               ├─ yield()
    │                               │     └─ swtch(&a.ctx, &sched_ctx)
    │<─────────────────────────────-┘
    │  (scheduler resumes here)
    ├─ a.state = RUNNABLE
    └─ ... next iteration picks task b
```

### swtch.S

`swtch(from, to)` saves `ra, sp, s0-s11` into `*from` and loads them from `*to`, then executes `ret`. Only callee-saved registers are involved : by the time `yield()` calls `swtch()`, the task has already handled any caller-saved values it needed.

The key insight: saving the return address (`ra`) means that when the task is resumed, `ret` lands back inside `yield()` right after the `swtch()` call, and `yield()` returns normally into the task's while loop.

---

## Files at a Glance

| File | Lines | What it does |
|---|---|---|
| `boot/start.S` | ~30 | Assembly entry point, minimal CPU setup |
| `kernel/entry.c` | ~35 | `main()`, task functions `a()` and `b()` |
| `kernel/uart.c` | ~55 | 16550 UART: `putc`, `puts`, `putx` |
| `kernel/kalloc.c` | ~30 | Bump page allocator |
| `kernel/vm.c` | ~85 | Sv39: `walk`, `mappages`, `kvminit`, `kvminithart` |
| `kernel/kerneltrap.c` | ~60 | Trap dispatch + CLINT timer setup |
| `kernel/trap.S` | ~65 | Save/restore all registers, call `kerneltrap` |
| `kernel/swtch.S` | ~55 | Save/restore callee-saved registers only |
| `kernel/proc.c` | ~55 | Task table, `scheduler()`, `yield()` |
| `kernel/panic.c` | ~10 | Print and halt |
| `include/proc.h` | ~45 | `struct context`, `struct task`, `enum taskstate` |
| `include/types.h` | ~20 | Basic integer types, `pte`, `pagetable` |
| `include/defs.h` | ~35 | All function declarations |
| `linker.ld` | ~55 | Section layout, stack, `end` symbol |
