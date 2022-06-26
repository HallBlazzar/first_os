#ifndef FIRSTOS_MEMORY_H
#define FIRSTOS_MEMORY_H

#include <stddef.h>

void* memset(void* ptr, int c, size_t size);
int memcmp(void* source1, void* source2, int count);
void* memcpy(void* dest, void* src, int len);

#endif
