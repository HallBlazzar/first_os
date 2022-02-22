#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

// void* -> can be converted any other pointer type without explicit cast
void* memset(void* ptr, int c, size_t size);

#endif