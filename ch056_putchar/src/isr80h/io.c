#include "io.h"
#include "task/task.h"
#include "kernel.h"
#include "keyboard/keyboard.h"

void* system_call_1_print(struct interrupt_frame* interrupt_frame) {
    void* user_space_message_buffer = get_task_stack_item(get_current_task(), 0); // the item is in task stack
    char kernel_buffer[1024];
    copy_string_from_task(get_current_task(), user_space_message_buffer, kernel_buffer, sizeof(kernel_buffer)); // copy that to kernel space
    print(kernel_buffer);
    return 0;
}

void* system_call_2_get_key(struct interrupt_frame* interrupt_frame) {
    char c = pop_input_character();
    return (void*)((int)c);
}

void* system_call_3_putchar(struct interrupt_frame* interrupt_frame) {
    char c = (char)(int) get_task_stack_item(get_current_task(), 0);
    write_char_on_terminal_to_final_char(c, 15);
    return 0;
}