#ifndef FIRST_OS_CLASSIC_H
#define FIRST_OS_CLASSIC_H

#define PS2_PORT 0x64
#define ENABLE_FIRST_PS2_PORT_COMMAND 0xAE

struct keyboard* get_classic_keyboard();

#endif
