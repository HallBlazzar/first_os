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
    void* program_data_pointer = kzalloc(stat.file_size);
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
            align_address(phdr_physical_address + phdr->p_filesz),
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
        return 0;
    }

    process->allocations[index] = ptr;

    return ptr;
}

static int find_free_allocation_index(struct process* process) {
    int result = -NO_FREE_MEM_ERROR;

    for (int i = 0; i < MAX_MEMORY_ALLOCATION; i++) {
        if (process->allocations[i] == 0) {
            result = i;
            break;
        }
    }

    return result;
}

void process_free(struct process* process, void* ptr) {
    if (!check_is_ptr_in_process_memory(process, ptr)) {
        return; // do nothing if ptr doesn't belong to process
    }

    unjoin_allocation(process, ptr);
    kfree(ptr);
}

static bool check_is_ptr_in_process_memory(struct process* process, void* ptr) {
    for (int i=0; i < MAX_MEMORY_ALLOCATION; i++) {
        if (process->allocations[i] == ptr) {
            return true;
        }
    }

    return false;
}

static void unjoin_allocation(struct process* process, void* ptr) {
    for (int i=0; i < MAX_MEMORY_ALLOCATION; i++) {
        if (process->allocations[i] == ptr) {
            process->allocations[i] = 0x00;
        }
    }
}