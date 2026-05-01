#include "defs.h"

#define MAX_TASKS 16

struct task tasks[MAX_TASKS];
int current_task = 0;
struct context scheduler_context;

void task_init(int id, void (*fn)(void), char *name){
    tasks[id].state = RUNNABLE;
    tasks[id].name = name;
    tasks[id].stack = (uint64)kalloc();
    tasks[id].context.sp = tasks[id].stack + 4096;
    tasks[id].context.ra = (uint64)fn;
}

void scheduler(){
    while (1){
        for(int i = 0; i < MAX_TASKS; i++){
            if(tasks[i].state == RUNNABLE){
                current_task = i;
                tasks[i].state = RUNNING;
                swtch(&scheduler_context, &tasks[i].context);
                tasks[i].state = RUNNABLE;
            }
        }
    }
}

void yield() {
    swtch(&tasks[current_task].context, &scheduler_context);
}