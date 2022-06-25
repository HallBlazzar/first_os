#include "misc.h"
#include "idt/idt.h"
#include "task/task.h"

void* system_call_0_sum(struct interrupt_frame* interrupt_frame) {
    int value2 = (int)get_task_stack_item(get_current_task(), 1);
    int value1 = (int)get_task_stack_item(get_current_task(), 0);
    return (void*)(value1 + value2);
}