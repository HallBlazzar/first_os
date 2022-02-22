#ifndef KHEAP_H
#define KHEAP_H

#include <stdint.h>
#include <stddef.h>

void initialize_kheap();
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif