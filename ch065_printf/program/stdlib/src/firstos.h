#ifndef FIRSTOS_H
#define FIRSTOS_H

#include <stddef.h>
#include <stdbool.h>

void print(const char* message);
int firstos_get_key();
int firstos_get_key_block();
void* firstos_malloc(size_t size);
void firstos_free(void* ptr);
void firstos_putchar(char c);
void firstos_readline(char* out, int max_length, bool show_output_while_typing);

#endif