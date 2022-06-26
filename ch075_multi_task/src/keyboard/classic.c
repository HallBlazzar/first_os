#include "keyboard.h"
#include "io/io.h"
#include "classic.h"
#include "kernel.h"
#include "idt/idt.h"
#include "task/task.h"
#include <stdint.h>
#include <stddef.h>

#define CAPS_LOCK_KEY 0x3A

int initialize_classic_keyboard();
void handle_classic_keyboard_interrupt();

static uint8_t keyboard_scan_set[] = {
    0x00, 0x1B, '1', '2', '3', '4', '5',
    '6','7', '8', '9', '0', '-', '=',
    0x08, '\t', 'Q', 'W', 'E', 'R', 'T',
    'Y', 'U', 'I','O', 'P', '[', ']',
    0X0D, 0X00, 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ';', '\'', '`',
    0X00,'\\', 'Z', 'X', 'C', 'V', 'B',
    'N', 'M', ',', '.','/', 0x00, '*',
    0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, '7', '8', '9', '-', '4', '5',
    '6', '+', '1', '2', '3', '0', '.'
};

// PS/2 keyboard https://wiki.osdev.org/PS/2_Keyboard
struct keyboard classic_keyboard = {
    .name = {"Classic"},
    .init = initialize_classic_keyboard
};

int initialize_classic_keyboard() {
    // initialize ps/2 keyboard
    // https://wiki.osdev.org/%228042%22_PS/2_Controller
    // port 0x64 --> write to command register
    // command 0xAE --> Enable first PS/2 port
    register_interrupt_callback(ISR_KEYBOARD_INTERRUPT, handle_classic_keyboard_interrupt);
    set_caps_lock(&classic_keyboard, CAPS_LOCK_OFF);
    outb(PS2_PORT, ENABLE_FIRST_PS2_PORT_COMMAND);
    return 0;
}

uint8_t convert_classic_keyboard_scan_code_to_char(uint8_t scan_code) {
    size_t keyboard_scan_set_size = sizeof(keyboard_scan_set) / sizeof(uint8_t); // get array size
    if (scan_code > keyboard_scan_set_size) {
        return 0;
    }

    char c = keyboard_scan_set[scan_code];
    if (get_caps_lock(&classic_keyboard) == CAPS_LOCK_OFF) {
        if (c >= 'A' && c <= 'Z') {
            c += 32;
        }
    }
    return c;
}

void handle_classic_keyboard_interrupt() {
    load_kernel_page();
    uint8_t scan_code = 0;
    scan_code = insb(KEYBOARD_INPUT_PORT);
    insb(KEYBOARD_INPUT_PORT); // ignore 1 byte

    if (scan_code & CLASSIC_KEYBOARD_KEY_RELEASED) {
        return; // do nothing if key released
    }

    if (scan_code == CAPS_LOCK_KEY) {
        CAPS_LOCK_STATE previous_state = get_caps_lock(&classic_keyboard);
        set_caps_lock(&classic_keyboard, previous_state == CAPS_LOCK_ON ? CAPS_LOCK_OFF : CAPS_LOCK_ON);
    }

    uint8_t c = convert_classic_keyboard_scan_code_to_char(scan_code);

    // get something meaningful
    if (c != 0) {
        push_input_character(c);
    }
}

struct keyboard* get_classic_keyboard() {
    return &classic_keyboard;
}