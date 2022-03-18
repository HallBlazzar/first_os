#include "task.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "kernel.h"
#include "status.h"
#include "process.h"

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
    task->registers.ss = USER_DATA_SEGMENT;
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
    switch_current_page_table_directory(task->page_directory->directory_entry);
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