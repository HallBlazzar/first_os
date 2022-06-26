#include "keyboard.h"
#include "status.h"
#include "kernel.h"
#include "task/process.h"
#include "task/task.h"
#include "classic.h"

static struct keyboard* keyboard_list_head = 0;
static struct keyboard* keyboard_list_last = 0;

static int get_tail_index_of_buffer(struct process* process);

void initialize_keyboard() {
    insert_keyboard(get_classic_keyboard());
}

int insert_keyboard(struct keyboard* keyboard) {
    int result = 0;
    if (keyboard->init == 0) {
        result = -INVALID_ARG_ERROR;
        goto out;
    }

    if (keyboard_list_last) {
        keyboard_list_last->next = keyboard;
        keyboard_list_last = keyboard;
    } else {
        keyboard_list_head = keyboard;
        keyboard_list_last = keyboard;
    }

    result = keyboard->init();
out:
    return result;
}

void push_input_character(char c) {
    struct process* process = get_current_process();
    if (!process) {
        return; // no process exist, no character allowed
    }

    if (c == 0) {
        return; // no character is not allowed
    }

    int real_index = get_tail_index_of_buffer(process);
    process->keyboard.buffer[real_index] = c;
    process->keyboard.tail ++;
}

static int get_tail_index_of_buffer(struct process* process) {
    return process->keyboard.tail % sizeof(process->keyboard.buffer);
}

void get_backspace(struct process* process) {
    process->keyboard.tail -= 1;
    int real_index = get_tail_index_of_buffer(process);
    process->keyboard.buffer[real_index] = 0x00;
}

char pop_input_character() {
    if (!get_current_task()) {
        return 0;
    }

    struct process* process = get_current_task()->process;
    int real_index = process->keyboard.head % sizeof(process->keyboard.buffer);
    char c = process->keyboard.buffer[real_index];

    if (c == 0x00) {
        return 0; // nothing to pop
    }

    process->keyboard.buffer[real_index] = 0;
    process->keyboard.head ++;
    return c;
}

void set_caps_lock(struct keyboard* keyboard, CAPS_LOCK_STATE state) {
    keyboard->caps_lock_state = state;
}

CAPS_LOCK_STATE get_caps_lock(struct keyboard* keyboard) {
    return keyboard->caps_lock_state;
}