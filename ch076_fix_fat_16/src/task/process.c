#include "process.h"
#include "config.h"
#include "status.h"
#include "memory/memory.h"
#include "task/task.h"
#include "memory/heap/kheap.h"
#include "fs/file.h"
#include "string/string.h"
#include "kernel.h"
#include "memory/paging/paging.h"
#include "loader/formats/elfloader.h"

// Current running process
struct process* current_process = 0;

static struct process* processes[MAX_PROCESSES] = {};

int get_free_slot();

static int load_process_data(const char* filename, struct process* process);
static int load_elf(const char* filename, struct process* process);
static int load_binary(const char* filename, struct process* process);
int map_process_memory(struct process* process);
int map_elf(struct process* process);
int map_binary(struct process* process);

static int find_free_allocation_index(struct process* process);
static void unjoin_allocation(struct process* process, void* ptr);

static bool check_is_ptr_in_process_memory(struct process* process, void* ptr);

int count_command_arguments(struct command_argument* root_argument);

int terminate_process_allocations(struct process* process);
int free_process_program_data(struct process* process);
int free_process_binary_data(struct process* process);
int free_process_elf_data(struct process* process);
static void unlink_process(struct process* process);
void switch_process_to_any();

static void initialize_process(struct process* process) {
    memset(process, 0, sizeof(struct process));
}

struct process* get_current_process() {
    return current_process;
}

int load_process(const char* filename, struct process** process) {
    int result = 0;

    int slot = get_free_slot();
    if (slot < 0) {
        result = -IS_TAKEN_ERROR;
        goto out;
    }

    result = load_process_into_slot(filename, process, slot);

out:
    return result;
}

int get_free_slot() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i] == 0)
            return i;
    }

    return -IS_TAKEN_ERROR;
}

// Load given filename as process into processes array
// process: process' address
// process_slot: index of the processes array
int load_process_into_slot(const char* filename, struct process** process, int process_slot) {
    int result = 0;
    struct task* task = 0;
    struct process* _process;
    void* program_stack_pointer = 0;

    if (get_process(process_slot) != NULL) {
        result = -IS_TAKEN_ERROR;
        goto out;
    }

    // Load process dat from file
    _process = kzalloc(sizeof(struct process));
    if (! _process) {
        result = -NO_FREE_MEM_ERROR;
        goto out;
    }

    initialize_process(_process);
    result = load_process_data(filename, _process);
    if (result < 0) {
        goto out;
    }

    // Allocate stack
    program_stack_pointer = kzalloc(USER_PROGRAM_STACK_SIZE);
    if (!program_stack_pointer) {
        result = -NO_FREE_MEM_ERROR;
        goto out;
    }

    strcpy_max_length(_process->filename, filename, sizeof(_process->filename));

    _process->stack = program_stack_pointer;
    _process->id = process_slot;

    // Create task
    task = new_task(_process);
    if (INT_ERROR(task) == 0) {
        result = INT_ERROR(task);
        goto out;
    }

    _process->task = task;

    result = map_process_memory(_process);
    if (result < 0) {
        goto out;
    }

    *process = _process;

    // Add process to processes array
    processes[process_slot] = _process;

out:
    if (IS_ERROR(result)) {
        if (_process && _process->task) {
            free_task(_process->task);
        }
        // free the whole process memory
        kfree(_process);
    }
    return result;
}

struct process* get_process(int process_id) {
    if (process_id < 0 || process_id >= MAX_PROCESSES) {
        return NULL;
    }
    return processes[process_id];
}

static int load_process_data(const char* filename, struct process* process) {
    int result = 0;

    result = load_elf(filename, process);

    if (result == -INVALID_FORMAT_ERROR) {
        result = load_binary(filename, process);
    }

    return result;
}

static int load_elf(const char* filename, struct process* process) {
    int result = 0;

    struct elf_file* elf_file = 0;

    result = load_elf_file(filename, &elf_file);

    if (IS_ERROR(result)) {
        goto out;
    }

    process->file_type = PROCESS_FILE_TYPE_ELF;
    process->elf_file = elf_file;

out:
    return result;
}

static int load_binary(const char* filename, struct process* process) {
    int result = 0;
    void* program_data_pointer = 0x00;

    int file_descriptor = fopen(filename, "r");
    if (!file_descriptor) {
        result = -IO_ERROR;
        goto out;
    }

    struct file_stat stat;
    result = fstat(file_descriptor, &stat);
    if (result != ALL_OK) {
        goto out;
    }

    // allocate file size as program data memory space
    program_data_pointer = kzalloc(stat.file_size);
    if (!program_data_pointer) {
        result = -NO_FREE_MEM_ERROR;
        goto out;
    }

    // read file_size * 1 blocks = [file size] blocs
    if (fread(program_data_pointer, stat.file_size, 1, file_descriptor) != 1) {
        result = -IO_ERROR;
        goto out;
    }

    process->file_type = PROCESS_FILE_TYPE_BINARY;
    process->process_data = program_data_pointer;
    process->size = stat.file_size;

out:
    if (result < 0) {
        if (program_data_pointer) {
            kfree(program_data_pointer);
        }
    }
    fclose(file_descriptor);
    return result;
}

int map_process_memory(struct process* process) {
    int result = 0;

    switch (process->file_type) {
        case PROCESS_FILE_TYPE_ELF:
            result = map_elf(process);
            break;
        case PROCESS_FILE_TYPE_BINARY:
            result = map_binary(process);
            break;
        default:
            panic("map_process_memory: invalid file type\n");
    }
    
    if (result < 0) {
        goto out;
    }

    // allow process to write to stack
    // map stack end --> stack grows "down"
    map_page(process->task->page_directory, (void*) END_ADDRESS_OF_PROGRAM_VIRTUAL_STACK_ADDRESS, process->stack, align_address(process->stack + USER_PROGRAM_STACK_SIZE), PAGING_IS_PRESENT | PAGING_IS_WRITABLE | PAGING_ACCESS_FROM_ALL);

out:
    return result;
}

int map_elf(struct process* process) {
    int result = 0;

    struct elf_file* elf_file = process->elf_file;
    struct elf_header* elf_header = get_elf_header(elf_file);
    struct elf32_phdr* elf_phdrs = get_elf_pheader(elf_header);

    for (int i = 0; i < elf_header->e_phnum; i++) {
        struct elf32_phdr* phdr = &elf_phdrs[i];
        void* phdr_physical_address = get_elf_physical_address(elf_file, phdr);
        int flags = PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL; // default flags

        // if header shows the memory of page is writable, additionally set the page writable
        if (phdr->p_flags & PF_W) {
            flags |= PAGING_IS_WRITABLE;
        }

        result = map_page(
            process->task->page_directory,
            align_paging_to_lower_page((void*)phdr->p_vaddr),
            align_paging_to_lower_page(phdr_physical_address),
            align_address(phdr_physical_address + phdr->p_memsz), // originally p_filesz, but in some case this field will be zero. It can cause page allocated incorrectly, which leads page fault.
            flags
        );

        if (IS_ERROR(result)) {
            break;
        }
    }

    return result;
}

int map_binary(struct process* process) {
    int result = 0;

    map_page(
        process->task->page_directory,
        (void*) PROGRAM_VIRTUAL_ADDRESS,
        process->process_data,
        align_address(process->process_data + process->size),
        PAGING_IS_PRESENT | PAGING_IS_WRITABLE | PAGING_ACCESS_FROM_ALL
    );

    return result;
}

int switch_process(struct process* process) {
    current_process = process;
    return 0;
}

int load_and_switch_process(const char* filename, struct process** process) {
    int result = load_process(filename, process);

    if (result == 0) {
        switch_process(*process);
    }

    return result;
}

void* process_malloc(struct process* process, size_t size) {
    void* ptr = kzalloc(size);

    if (!ptr) {
        return 0;
    }

    int index = find_free_allocation_index(process);

    if (index < 0) {
        goto out_error;
    }

    // map virtual address to the same physical address in page table
    // then make page accessible by all ring level
    // otherwise the memory allocated only accessible to kernel,
    // user program is not allowed --> give the memory PAGING_ACCESS_FROM_ALL privilege
    int result = map_page(
        process->task->page_directory, ptr, ptr,
        align_address(ptr + size),
        PAGING_IS_WRITABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL
    );
    
    if (result < 0) {
        goto out_error;
    }

    process->allocations[index].ptr = ptr;
    process->allocations[index].size = size;

    return ptr;

out_error:
    if (ptr) {
        kfree(ptr);
    }

    return 0;
}

static int find_free_allocation_index(struct process* process) {
    int result = -NO_FREE_MEM_ERROR;

    for (int i = 0; i < MAX_MEMORY_ALLOCATION; i++) {
        if (process->allocations[i].ptr == 0) {
            result = i;
            break;
        }
    }

    return result;
}

void process_free(struct process* process, void* ptr) {
    struct process_allocation* allocation = get_allocation_by_address(process, ptr);

    if (!allocation) {
        // not process pointer
        return;
    }

    // in general, after unallocation, page also need to be removed from page table entry
    // and the memory should be turned into supervisor mode
    // these are to prevent the original process can still access the memory after freeing
    // It's important if other process get the memory, then the one owns it before can never access data in it.

    // unmap memory
    int result = map_page(
        process->task->page_directory,
        allocation->ptr,
        allocation->ptr,
        align_address(allocation->ptr + allocation->size),
        0x00
    );

    if (result < 0) {
        return;
    }

    unjoin_allocation(process, ptr);

    kfree(ptr);
}

static bool check_is_ptr_in_process_memory(struct process* process, void* ptr) {
    for (int i=0; i < MAX_MEMORY_ALLOCATION; i++) {
        if (process->allocations[i].ptr == ptr) {
            return true;
        }
    }

    return false;
}

static void unjoin_allocation(struct process* process, void* ptr) {
    for (int i=0; i < MAX_MEMORY_ALLOCATION; i++) {
        if (process->allocations[i].ptr == ptr) {
            process->allocations[i].ptr = 0x00;
            process->allocations[i].size = 0;
        }
    }
}

static struct process_allocation* get_allocation_by_address(struct process* process, void* address) {
    for (int i=0; i < MAX_MEMORY_ALLOCATION; i++) {
        if (process->allocations[i].ptr == address) {
            return &process->allocations[i];
        }
    }

    return 0;
}

void get_process_arguments(struct process* process, int* argc, char*** argv) {
    *argc = process->arguments.argc;
    *argv = process->arguments.argv;
}

int inject_process_arguments(struct process* process, struct command_argument* root_argument) {
    int result = 0;
    struct command_argument* current = root_argument;
    int i = 0;
    int argc = count_command_arguments(root_argument);
    if (argc == 0) {
        result = -IO_ERROR;
        goto out;
    }

    char **argv = process_malloc(process, sizeof(const char*) * argc);
    if (!argv) {
        result = -NO_FREE_MEM_ERROR;
        goto out;
    }

    while (current) {
        char* argument_string = process_malloc(process, sizeof(current->argument));
        if (!argument_string) {
            result = -NO_FREE_MEM_ERROR;
            goto out;
        }
        strcpy_max_length(argument_string, current->argument, sizeof(current->argument));
        argv[i] = argument_string;
        current = current->next;
        i++;
    }

    process->arguments.argc = argc;
    process->arguments.argv = argv;

out:
    return result;
}

int count_command_arguments(struct command_argument* root_argument) {
    struct command_argument* current = root_argument;
    int i = 0;

    while(current) {
        i++;
        current = current->next;
    }

    return i;
}

int terminate_process(struct process* process) {
    int result = 0;

    // free pages
    result = terminate_process_allocations(process);
    if (result < 0) {
        goto out;
    }

    // free program data
    result = free_process_program_data(process);
    if (result < 0) {
        goto out;
    }

    // free stack memory
    kfree(process->stack);

    // free process task
    free_task(process->task);

    // remove process from process list
    unlink_process(process);

out:
    return result;
}

int terminate_process_allocations(struct process* process) {
    for (int i=0; i < MAX_MEMORY_ALLOCATION; i++) {
        process_free(process, process->allocations[i].ptr);
    }

    return 0;
}

int free_process_program_data(struct process* process) {
    int result = 0;
    switch(process->file_type) {
        case PROCESS_FILE_TYPE_BINARY:
            result = free_process_binary_data(process);
            break;
        case PROCESS_FILE_TYPE_ELF:
            result = free_process_elf_data(process);
            break;
        default:
            result = INVALID_ARG_ERROR;
            break;
    }
    return result;
}

int free_process_binary_data(struct process* process) {
    kfree(process->process_data);
    return 0;
}

int free_process_elf_data(struct process* process) {
    close_elf_file(process->elf_file);
    return 0;
}

static void unlink_process(struct process* process) {
    processes[process->id] = 0x00; // remove process from list

    if (current_process == process) {
        switch_process_to_any(); // change current process to anyone in the processes list
    }
}

void switch_process_to_any() {
    for (int i=0; i < MAX_PROCESSES; i++) {
        if (processes[i]) {
            switch_process(processes[i]);
            return;
        }
    }

    panic("No processes to switch\n");
}