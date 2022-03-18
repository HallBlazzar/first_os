#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "config.h"
#include "task.h"

struct process {
    // process ID
    uint16_t id;

    char filename[MAX_PATH];

    // main process task
    struct task* task;

    // record physical addresses requested by the process(malloc)
    // once program not exit normally or not free memory before exist
    // kernel can free them according tracking this array
    void* allocations[MAX_MEMORY_ALLOCATION];

    // physical pointer to process data
    // in this course, binary file will be loaded to execute
    // if use ELF, then it will be a pointer to an ELF structure
    void* process_data;

    // physical pointer to stack memory
    void* stack;

    // size of data pointed to by "process_memory"
    uint32_t size;
};

int load_process_into_slot(const char* filename, struct process** process, int process_slot);

#endif
