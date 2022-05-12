#include "task.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "kernel.h"
#include "status.h"
#include "process.h"
#include "idt/idt.h"
#include "string/string.h"
#include "loader/formats/elfloader.h"

// Current running task
struct task* current_task = 0;

// Task linked list
struct task* task_tail = 0;
struct task* task_head = 0;

int initialize_task(struct task* task, struct process* process);
static void remove_task_from_list(struct task* task);


struct task* get_current_task() {
    return current_task;
}

struct task* new_task(struct process* process) {
    int result = 0;
    struct task* task = kzalloc(sizeof(struct task));
    if (!task) {
        result = -NO_FREE_MEM_ERROR;
        goto out;
    }

    result = initialize_task(task, process);
    if (result != ALL_OK) {
        goto out;
    }

    if (task_head == 0) {
        task_head = task;
        task_tail = task;
        current_task = task;
        goto out;
    }

    task_tail -> next = task;
    task->prev = task_tail;
    task_tail = task;

out:
    if (IS_ERROR(result)) {
        free_task(task);
        return ERROR(result);
    }
    return task;
}

int initialize_task(struct task* task, struct process* process) {
    memset(task, 0, sizeof(struct task));
    task->page_directory = create_4gb_page(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    if (!task->page_directory) {
        return -IO_ERROR;
    }

    task->registers.ip = PROGRAM_VIRTUAL_ADDRESS;

    // instruction pointer should point to e_entry when executable is elf
    if (process -> file_type == PROCESS_FILE_TYPE_ELF) {
        task->registers.ip = get_elf_header(process->elf_file) -> e_entry;
    }

    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.cs = USER_CODE_SEGMENT;
    task->registers.esp = START_ADDRESS_OF_PROGRAM_VIRTUAL_STACK_ADDRESS;

    task->process = process;

    return 0;
}

int free_task(struct task* task) {
    free_4gb_page(task->page_directory);
    remove_task_from_list(task);

    kfree(task);

    return 0;
}

static void remove_task_from_list(struct task* task) {
    if (task -> prev) {
        task->prev->next = task->next;
    }

    if (task == task_head) {
        task_head = task->next;
    }

    if (task == task_tail) {
        task_tail = task->prev;
    }

    if (task == current_task) {
        current_task = get_next_task();
    }
}

struct task* get_next_task() {
    if (!current_task->next) {
        return task_head;
    }

    return current_task->next;
}

// change current context expected to run
// 1. change current running task
// 2. change page directory to current task
int switch_task(struct task* task) {
    current_task = task;
    switch_current_page_table_directory(task->page_directory);
    return 0;
}

// switch from kernel page directory to task page directory
int load_task_page() {
    change_to_user_data_register();
    switch_task(current_task);
    return 0;
}

void run_first_task() {
    if (!current_task) {
        panic("run_first_task(): current task doesn't exist");
    }

    switch_task(task_head);
    return_task(&task_head->registers); // enter the user land
}

void save_current_task_state(struct interrupt_frame* interrupt_frame) {
    if (!get_current_task()) {
        panic("No current task to save\n");
    }

    struct task* task_to_save = get_current_task();
    save_task_state(task_to_save, interrupt_frame);
}

void save_task_state(struct task* task_to_save, struct interrupt_frame* interrupt_frame) {
    task_to_save->registers.ip = interrupt_frame->ip;
    task_to_save->registers.cs = interrupt_frame->cs;
    task_to_save->registers.flags = interrupt_frame->flags;
    task_to_save->registers.esp = interrupt_frame->esp;
    task_to_save->registers.ss = interrupt_frame->ss;
    task_to_save->registers.eax = interrupt_frame->eax;
    task_to_save->registers.ebp = interrupt_frame->ebp;
    task_to_save->registers.ebx = interrupt_frame->ebx;
    task_to_save->registers.ecx = interrupt_frame->ecx;
    task_to_save->registers.edi = interrupt_frame->edi;
    task_to_save->registers.edx = interrupt_frame->edx;
    task_to_save->registers.esi = interrupt_frame->esi;
}

int copy_string_from_task(struct task* task, void* virtual_address, void* physical_address, int max_length) {
    if (max_length >= PAGE_SIZE) {
        return -INVALID_ARG_ERROR;
    }

    // allocate memory could be shared by both kernel and task
    int result = 0;
    char* temp = kzalloc(max_length);
    if (!temp) {
        result = -NO_FREE_MEM_ERROR;
        goto out;
    }

    uint32_t* task_directory = task->page_directory->directory_entry;
    uint32_t old_entry = get_page(task_directory, temp); // get memory address of temp(virtual) which will map to task directory(physical)
    map_single_page(task->page_directory, temp, temp, PAGING_IS_WRITABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL); // map physical address of temp to virtual address in task
    switch_current_page_table_directory(task->page_directory); // switch to task page to perform string copy, copy content to temp
    strcpy_max_length(temp, virtual_address, max_length);
    load_kernel_page(); // switch to kernel page

    result = set_page_table_entry(task_directory, temp, old_entry);

    if (result < 0) {
        result = -IO_ERROR;
        goto free;
    }

    strcpy_max_length(physical_address, temp, max_length); // copy content in temp to kernel page physical address

free:
    kfree(temp);

out:
    return result;
}

void* get_task_stack_item(struct task* task, int index) {
    void* result = 0;

    uint32_t* stack_pointer = (uint32_t*) task->registers.esp;

    // switch to given task's page
    load_given_task_page(task);

    result = (void*) stack_pointer[index];

    // switch to kernel page
    load_kernel_page();

    return result;
}

int load_given_task_page(struct task* task) {
    change_to_user_data_register();
    switch_current_page_table_directory(task->page_directory);
    return 0;
}