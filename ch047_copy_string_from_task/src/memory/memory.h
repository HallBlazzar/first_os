#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

// void* -> can be converted any other pointer type without explicit cast
void* memset(void* ptr, int c, size_t size);
int memcmp(void* source1, void* source2, int count);
void* memcpy(void* dest, void* src, int len);

#endif