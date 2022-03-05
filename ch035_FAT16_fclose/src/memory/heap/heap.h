#ifndef HEAP_H
#define HEAP_H
#include "config.h"
#include <stdint.h>
#include <stddef.h>

#define HEAP_BLOCK_TABLE_ENTRY_TAKEN 0x01
#define HEAP_BLOCK_TABLE_ENTRY_FREE 0x00

#define HEAP_BLOCK_HAS_NEXT 0b10000000
#define HEAP_BLOCK_IS_FIRST 0b01000000

typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

struct heap_table {
    HEAP_BLOCK_TABLE_ENTRY* entries;
    size_t total_entries_num;
};

struct heap {
    struct heap_table* table;
    void* start_address_of_heap;
};

int create_heap(struct heap* heap, void* start_address_of_heap, void* end_address_of_heap, struct heap_table* heap_table);
void* heap_malloc(struct heap* heap,size_t size);
void heap_free(struct heap* heap, void* address);

#endif