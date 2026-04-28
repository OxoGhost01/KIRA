#include "defs.h"

extern char end[];
static char *next = 0;

uint64 *kalloc(){
    if(next == 0){
        next = end;
    }

    uint64 *ret = (uint64*)next;
    next += 4096;
    return ret;
}
