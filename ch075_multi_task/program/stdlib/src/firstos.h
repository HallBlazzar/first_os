#ifndef FIRSTOS_H
#define FIRSTOS_H

#include <stddef.h>
#include <stdbool.h>

struct command_argument {
    char argument[512];
    struct command_argument* next;
};

struct process_arguments {
    int argc;
    char** argv;
};

struct command_argument* firstos_parse_command(const char* command, int max);

void print(const char* message);
int firstos_get_key();
int firstos_get_key_block();
void* firstos_malloc(size_t size);
void firstos_free(void* ptr);
void firstos_putchar(char c);
void firstos_readline(char* out, int max_length, bool show_output_while_typing);
void firstos_start_load_process(const char* filename);

void firstos_get_process_arguments(struct process_arguments* arguments);

int firstos_command(struct command_argument* arguments);
int firstos_command_run(const char* command);

void fistos_exit();

#endif