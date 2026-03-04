# This document is notes i take while learning how RISC-V works

## Registers

> https://www.youtube.com/watch?v=VEQL5bJeWB0&list=PLbtzT1TYeoMiKup6aoQc3V_d7OvOKc3P5


- 32 Genral Purpose Registers (GPRs)
  None have special functionality but programming conventions exist
- Program Counter (PC) : holds address of next instruction to execute
- Control and Status Registers (CSRs)

To remember : 
- bytes are 8 bits
- words are 32 bits

bytes :
- signed : -128 / +127
- unsigned : 0 / 255

Signed are recommended in general
Unsigned are used for memory addresses, pointers, bit vectors

### Example RV instruction

opcode | operands | comments
---|---|---
add | x20, x6, x7 | # Add contents of regs x6 and x7 and store in x20

### Registers names

register | name | function | details
---|---|---|---
x0 | zero | fixed reg | Hardwired to 0 (used as source or "trash")
x1 | ra | Return address | CALL and RET
x2 | sp | Stack pointer | Stack pointer (programming convention)
x3 | gp | Global pointer | Points area where global variables are stored (makes addressing easier)
x4 | tp | Thread pointer | Same as `gp` but variables are thread specific
x5 | t0 | Temporary register 0 | "Work" register
x6 | t1 | Temporary register 1 | Convention : can be modified by any function called "Caller saved"
x7 | t2 | Temporary register 2 | The called function has to save the value
x8 | s0 | Saved register 0 | "Work" register too
x9 | s1 | Saved register 1 | "Callee saved" -> cost (function must save the value before using it, and restore the value after)
x10 | a0 | Function argument 0 | Up to 8 arguments passed in registers
x11 | a1 | Function argument 1 | if more, then pass on the stack
x12 | a2 | Function argument 2 | if any result, return in a0
x13 | a3 | Function argument 3 | -
x14 | a4 | Function argument 4 | -
x15 | a5 | Function argument 5 | -
x16 | a6 | Function argument 6 | -
x17 | a7 | Function argument 7 | -
x18 | s2 | Saved register 2
x19 | s3 | Saved register 3
x20 | s4 | Saved register 4
x21 | s5 | Saved register 5
x22 | s6 | Saved register 6
x23 | s7 | Saved register 7
x24 | s8 | Saved register 8
x25 | s9 | Saved register 9
x26 | s10 | Saved register 10
x27 | s11 | Saved register 11
x28 | t3 | Temporary register 3
x29 | t4 | Temporary register 4
x30 | t5 | Temporary register 5
x31 | t6 | Temporary register 6

## ASM RISC-V

> https://www.youtube.com/watch?v=QiYGnDeNv0A&list=PLbtzT1TYeoMiKup6aoQc3V_d7OvOKc3P5&index=2

### Opcode example : 

```asm
my Loop: add t0,a0,a1  # add t0 = a0 + a1
```
- add   | machine instruction
- call  | pseudo instruction
- c.add | compressed instruction
- .word | assembler directive

### Labels

`myLabel:`

it's an identifier (alpha, digit, underscore) that can be used to jump to the address where the label is defined (colon in necessary)

### Notations for operands

rd -> Destination Reg
rs1, rs2 -> Source Regs
imm, imm12, off -> Immediate values
address -> Any address

Examples : 

```asm
addi rd,rs1,imm12   # rd = rs1 + imm
lw   rd,off(rs1)    # load wrod from memory
sw   rs2,off(rs1)   # store rs2 into memory (**STORE : Destination is listed last**)
```

### Instructions : 

`add` : addition between 2 reg values
`addi` : addition between a reg values and an immediate value (limited to 12 bits [-2048, 2047])
`sub` : substraction
`and` : logical and
`or` : logical or
`xor` : logical xor
`sll` : shift left logical
`srl` : shift right logical
`sra` : shift right arithmetic
`slt` : set less than
`sltu` : set less than unsigned

*Same for immediate values (execpt for subi)*

`lb` : load byte
`lh` : load halfword
`lw` : load word
`lbu` : load byte unsigned
`lhu` : load halfword unsigned

> `lb  rd,imm12(rs1)   # rd = MEM [rs1+imm12]`

`sb` : store byte
`sh` : store halfword
`sw` : store word

> `sb  rs2,imm12(rs1)  # MEM [rs1+imm12] = rs2`

# Compliling the first code

first of all we need some librairies : 

```bash
sudo apt install gcc-riscv64-unknown-elf qemu-system-misc gdb-multiarch
```

Then the first code here is a future kernel code : we do not include `stdio.h` here because it has nothing to do in a kernel we write.

And we compile with :

```bash
riscv64-unknown-elf-gcc hello.c -nostdlib -e _start -o hello.elf
```
Where :
- `hello.c` is the name of your file.
- `-nostdlib` is to disable the standard library.
- `-e _start` is to set the entry point to `_start`.
- `-o hello.elf` is to output the executable as `hello.elf`.

And we run it with `qemu-system-riscv64 -machine virt -nographic -kernel hello.elf`

# The linker

The linker is a tool used to combine multiple object files into a single executable file.

The important things to know : 
- you have `ENTRY()` which tells the linker where the entry point is. (the `main` function)
- then `SECTIONS`, in the section part, we define where are places our code and the variables (defined and non-defined).
    - `.text` for the compiled code (for now there's only this as we don't have any variables)
  
Then we define a symbol pointing to the end of the stack. (the top of the stack)

We test with `riscv64-unknown-elf-gcc -nostdlib -ffreestanding -T linker.ld hello.c -o kernel.elf` and `riscv64-unknown-elf-objdump -h kernel.elf`

This returns : 
```bash
kernel.elf:     file format elf64-littleriscv

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 .text         0000000a  0000000080000000  0000000080000000  00001000  2**1
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .comment      00000018  0000000000000000  0000000000000000  0000100a  2**0
                  CONTENTS, READONLY
  2 .riscv.attributes 0000005a  0000000000000000  0000000000000000  00001022  2**0
                  CONTENTS, READONLY
```

# Makefile

I know it's a bit dumb, but i never wrote a Makefile before, so i don't know the syntax.

It works like so :

```Makefile
target: dependencies
    command
```

The target is the file you wanna create, for example main.o
The dependencies are the files you need to create the target, for example main.c
The command is the command to create the target, for example `gcc main.c -c`
So here is would be :

```Makefile
main.o: main.c
    gcc main.c -c
```

Here you go ! Ez isn't it ?

Tips : 
You can create a variable with `myVariable = something` and use it with `$(myVariable)`.

> Note : to stop the qemu emulation, press `Ctrl+A` then `x`