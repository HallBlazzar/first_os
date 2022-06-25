#include "heap.h"
#include "task/task.h"
#include "task/process.h"
#include <stddef.h>

void* system_call_4_malloc(struct interrupt_frame* interrupt_frame) {
    size_t size = (int)get_task_stack_item(get_current_task(), 0);
    return process_malloc(get_current_task()->process, size); // ensure referring to the process of the task calls malloc
}

void* system_call_5_free(struct interrupt_frame* interrupt_frame) {
    void* ptr_to_free = get_task_stack_item(get_current_task(), 0);
    process_free(get_current_task()->process, ptr_to_free);
    return 0;
}