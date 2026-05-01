#ifndef PROC_H
#define PROC_H

#include "types.h"

struct context{
    uint64 ra;
    uint64 sp;
    uint64 s0;
    uint64 s1;
    uint64 s2;
    uint64 s3;
    uint64 s4;
    uint64 s5;
    uint64 s6;
    uint64 s7;
    uint64 s8;
    uint64 s9;
    uint64 s10;
    uint64 s11;
};

enum taskstate {
    UNUSED,
    RUNNABLE,
    RUNNING,
};

struct task{
    enum taskstate state;
    struct context context;
    uint64 stack;
    char *name;
};

#endif