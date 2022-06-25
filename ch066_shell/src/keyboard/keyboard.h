#ifndef KEYBOARD_H
#define KEYBOARD_H

struct process;

typedef int (*KEYBOARD_INIT_FUNCTION)();

struct keyboard {
    KEYBOARD_INIT_FUNCTION init;
    char name[20];
    struct keyboard* next;
};

void initialize_keyboard();
void push_input_character(char c);
void get_backspace(struct process* process);
char pop_input_character();
int insert_keyboard(struct keyboard* keyboard);

#endif