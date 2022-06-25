#include "firstos.h"

int firstos_get_key_block() {
    int value = 0;

    do {
        value = firstos_get_key();
    } while(value == 0); // keep reading until any key pressed

    return value;
}

void firstos_readline(char* out, int max_length, bool show_output_while_typing) {
    int i = 0;

    for (i = 0; i < max_length - 1; i++) {
        char key = firstos_get_key_block();

        // carriage return(Enter key)
        if (key == 13) {
            break;
        }

        if (show_output_while_typing) {
            firstos_putchar(key);
        }

        // backspace
        if (key == 0x08 && i >= 1) {
            out[i - 1] = 0x00;
            // back to one prior than previous char
            i -= 2;
            continue;
        }

        out[i] = key;
    }

    // terminator char
    out[i] = 0x00;
}