#include "io.h"
#include "task/task.h"
#include "kernel.h"

void* system_call_1_print(struct interrupt_frame* interrupt_frame) {
    void* user_space_message_buffer = get_task_stack_item(get_current_task(), 0); // the item is in task stack
    char kernel_buffer[1024];
    copy_string_from_task(get_current_task(), user_space_message_buffer, kernel_buffer, sizeof(kernel_buffer)); // copy that to kernel space
    print(kernel_buffer);
    return 0;
}