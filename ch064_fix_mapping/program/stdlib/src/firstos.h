#ifndef FIRSTOS_H
#define FIRSTOS_H

#include <stddef.h>

void print(const char* message);
int get_key();
void* firstos_malloc(size_t size);
void firstos_free(void* ptr);

#endif