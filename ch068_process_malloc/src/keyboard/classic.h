#ifndef FIRST_OS_CLASSIC_H
#define FIRST_OS_CLASSIC_H

#define PS2_PORT 0x64
#define ENABLE_FIRST_PS2_PORT_COMMAND 0xAE

#define CLASSIC_KEYBOARD_KEY_RELEASED 0x80
#define ISR_KEYBOARD_INTERRUPT 0x21
#define KEYBOARD_INPUT_PORT 0x60

struct keyboard* get_classic_keyboard();

#endif
