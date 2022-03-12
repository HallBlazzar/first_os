#ifndef TASK_H
#define TASK_H

#include "config.h"
#include "memory/paging/paging.h"

// when interrupt occur, current process's context(registers values) will be stored for resuming
struct registers {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t ip; // program counter. record current address the task is paused(last executed instruction)
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
};

// liked list to record tasks
struct task {
    struct paging_4gb_chunk* page_directory; // task's page directory
    struct registers registers; // task's registers' value when it is interrupted
    struct task* next;
    struct task* prev;
};

struct task* new_task();
struct task* get_current_task();
struct task* get_next_task();
int free_task(struct task* task);

#endif