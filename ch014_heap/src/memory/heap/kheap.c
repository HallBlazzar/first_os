#include "kheap.h"
#include "heap.h"
#include "config.h"
#include "kernel.h"

struct heap kernel_heap;
struct heap_table kernel_heap_table;

void initialize_kheap();

//create 100Mb heap, with 4096 block size
//total number of blocks = (1024*1024*100) / 4096 = 25600 -> number of entries
void initialize_kheap() {
    int total_table_entries = HEAP_SIZE_BYTES / HEAP_BLOCK_SIZE;
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*) HEAP_TABLE_ADDRESS; // check https://wiki.osdev.org/Memory_Map_(x86)
    kernel_heap_table.total_entries_num = total_table_entries;

    void* end_address_of_heap = (void*)(HEAP_ADDRESS + HEAP_SIZE_BYTES);
    int result = create_heap(&kernel_heap, (void*)HEAP_ADDRESS, end_address_of_heap, &kernel_heap_table);

    if (result < 0) {
        print("Failed to create heap\n");
    }
}

void* kmalloc(size_t size) {
    return heap_malloc(&kernel_heap, size);
}

void kfree(void* address) {
    heap_free(&kernel_heap, address);
}