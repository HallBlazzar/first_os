#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "task.h"

#define PROCESS_FILE_TYPE_ELF 0
#define PROCESS_FILE_TYPE_BINARY 1

typedef unsigned char PROCESS_FILE_TYPE;

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

    PROCESS_FILE_TYPE file_type;

    union {
        // physical pointer to process data
        // in this course, binary file will be loaded to execute
        // if use ELF, then it will be a pointer to an ELF structure
        void* process_data;
        struct elf_file* elf_file;
    };

    // physical pointer to stack memory
    void* stack;

    // size of data pointed to by "process_memory"
    uint32_t size;

    struct keyboard_buffer {
        char buffer[KEYBOARD_BUFFER_SIZE];
        int tail;
        int head;
    } keyboard;
};

int load_process_into_slot(const char* filename, struct process** process, int process_slot);
int load_and_switch_process(const char* filename, struct process** process);
int load_process(const char* filename, struct process** process);
int switch_process(struct process* process);
struct process* get_current_process();
struct process* get_process(int process_id);
void* process_malloc(struct process* process, size_t size);
void process_free(struct process* process, void* ptr);

#endif
