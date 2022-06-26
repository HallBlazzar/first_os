#ifndef KEYBOARD_H
#define KEYBOARD_H

#define CAPS_LOCK_ON 1
#define CAPS_LOCK_OFF 0

typedef int CAPS_LOCK_STATE;

struct process;

typedef int (*KEYBOARD_INIT_FUNCTION)();

struct keyboard {
    KEYBOARD_INIT_FUNCTION init;
    char name[20];
    CAPS_LOCK_STATE  caps_lock_state;
    struct keyboard* next;
};

void initialize_keyboard();
void push_input_character(char c);
void get_backspace(struct process* process);
char pop_input_character();
int insert_keyboard(struct keyboard* keyboard);
void set_caps_lock(struct keyboard* keyboard, CAPS_LOCK_STATE state);
CAPS_LOCK_STATE get_caps_lock(struct keyboard* keyboard);

#endif