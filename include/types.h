#ifndef TYPES_H
#define TYPES_H

/* Basic integer type aliases.
 *
 * We avoid depending on stdint.h : there is no libc in kernel space.
 * These match the RISC-V LP64 ABI: int = 32 bits, long = 64 bits.
 */
typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long  uint64;

/* Page table types for Sv39 virtual memory.
 *
 * pte       = one 64-bit page table entry
 * pagetable = pointer to an array of 512 PTEs (one 4 KB page worth)
 */
typedef uint64  pte;
typedef uint64 *pagetable;

#endif
