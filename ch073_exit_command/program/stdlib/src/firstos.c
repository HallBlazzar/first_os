#include "firstos.h"
#include "string.h"

struct command_argument* firstos_parse_command(const char* command, int max) {
    struct command_argument* root_command = 0;
    char source_command[1025];

    if (max >= (int) sizeof(source_command)) {
        return 0;
    }

    strcpy_max_length(source_command, command, sizeof(source_command));
    char* token = strtok(source_command, " ");
    print(token);
    if (!token) {
        goto out;
    }

    root_command = firstos_malloc(sizeof(struct command_argument));
    if (!root_command) {
        goto out;
    }

    strcpy_max_length(root_command->argument, token, sizeof(root_command->argument));
    root_command->next = 0;

    struct command_argument* current = root_command;
    token = strtok(NULL, " ");

    while (token != 0) {
        struct command_argument* new_command = firstos_malloc(sizeof(struct command_argument));
        if (!new_command) {
            break;
        }
        strcpy_max_length(new_command->argument, token, sizeof(new_command->argument));
        new_command->next = 0x00;
        current->next = new_command;
        current = new_command;
        token = strtok(NULL, " ");
    }

out:
    return root_command;
}

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

int firstos_command_run(const char* command) {
    char buffer[1024];
    strcpy_max_length(buffer, command, sizeof(buffer)); // set maximum size of command to read
    struct command_argument* root_command_argument = firstos_parse_command(buffer, sizeof(buffer));
    if (!root_command_argument) {
        return -1; // invalid argument
    }
    return firstos_command(root_command_argument);
}