#include "heap.h"
#include "config.h"
#include "kernel.h"
#include "status.h"
#include "memory/memory.h"
#include <stdbool.h>


static bool validate_heap_alignment(void* address);
static int validate_heap_table(void* start_address_of_heap, void* end_address_of_heap, struct heap_table* heap_table);

static uint32_t align_heap_value_to_upper(uint32_t value);
void* heap_malloc_blocks(struct heap* heap, uint32_t total_blocks);
int get_start_heap_block(struct heap* heap, uint32_t total_blocks);
static int get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry);
void* convert_heap_block_to_address(struct heap* heap, uint32_t block);
void mark_heap_blocks_taken(struct heap* heap, uint32_t start_block, uint32_t total_blocks);

int heap_address_to_block(struct heap* heap, void* address);
void mark_heap_blocks_free(struct heap* heap, int start_block);

int create_heap(struct heap* heap, void* start_address_of_heap, void* end_address_of_heap, struct heap_table* heap_table) {
    int result = 0;

    if (!validate_heap_alignment(start_address_of_heap) || !validate_heap_alignment(end_address_of_heap)) {
        result = -INVALID_ARG_ERROR;
        goto out;
    }

    memset(heap, 0, sizeof(struct heap));
    heap->start_address_of_heap = start_address_of_heap;
    heap->table = heap_table;
    result = validate_heap_table(start_address_of_heap, end_address_of_heap, heap_table);

    if (result < 0) {
        goto out;
    }

    size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * heap_table->total_entries_num;
    memset(heap_table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);

out:
    return result;
}

static bool validate_heap_alignment(void* address) {
    return ((unsigned int)address % HEAP_BLOCK_SIZE) == 0;
}

static int validate_heap_table(void* start_address_of_heap, void* end_address_of_heap, struct heap_table* heap_table) {
    int result = 0;

    size_t heap_size = (size_t)(start_address_of_heap - end_address_of_heap);
    size_t total_blocks = heap_size / HEAP_BLOCK_SIZE;

    if (heap_table->total_entries_num != total_blocks) {
        result = INVALID_ARG_ERROR;
        goto out;
    }

out:
    return result;
}


void* heap_malloc(struct heap* heap, size_t size) {
    size_t aligned_size = align_heap_value_to_upper(size);
    uint32_t total_blocks = aligned_size / HEAP_BLOCK_SIZE;
    return heap_malloc_blocks(heap, total_blocks);
}

static uint32_t align_heap_value_to_upper(uint32_t value) {
    if (value % HEAP_BLOCK_SIZE == 0) {
        return value;
    }

    value = (value - (value % HEAP_BLOCK_SIZE));
    value += HEAP_BLOCK_SIZE;

    return value;
}

void* heap_malloc_blocks(struct heap* heap, uint32_t total_blocks) {
    void* address = 0;

    int start_block = get_start_heap_block(heap, total_blocks);

    if (start_block < 0) {
        goto out;
    }

    address = convert_heap_block_to_address(heap, start_block);

    // Mark blocks as taken
    mark_heap_blocks_taken(heap, start_block, total_blocks);

out:
    return address;
}

int get_start_heap_block(struct heap* heap, uint32_t total_blocks) {
    struct heap_table* heap_table = heap->table;
    int current_block = 0;
    int start_block = -1;

    for (size_t i = 0; i < heap_table->total_entries_num; i++) {
        if (get_entry_type(heap_table->entries[i]) != HEAP_BLOCK_TABLE_ENTRY_FREE) {
            current_block = 0;
            start_block = -1;
            continue;
        }

        if (start_block == -1) {
            start_block = i;
        }
        current_block++;

        if (current_block == total_blocks) {
            break;
        }
    }

    if ( (start_block == -1) || (current_block < total_blocks) ) {
        return -NO_FREE_MEM_ERROR;
    }

    return start_block;
}

static int get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry) {
    return entry & 0x0f; // last(right most) 4 bits
}

void* convert_heap_block_to_address(struct heap* heap, uint32_t block) {
    return heap->start_address_of_heap + (block * HEAP_BLOCK_SIZE);
}

void mark_heap_blocks_taken(struct heap* heap, uint32_t start_block, uint32_t total_blocks) {
    int end_block = (start_block + total_blocks) - 1;

    HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;

    if (total_blocks > 1) {
        entry |= HEAP_BLOCK_HAS_NEXT;
    }

    for (int i = start_block; i <= end_block ; i++) {
        heap->table->entries[i] = entry;
        entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;
        if (i != end_block - 1) {
            entry |= HEAP_BLOCK_HAS_NEXT;
        }
    }
}

void heap_free(struct heap* heap, void* address) {
    mark_heap_blocks_free(heap, heap_address_to_block(heap, address));
}

int heap_address_to_block(struct heap* heap, void* address) {
    return ((int)(address - heap->start_address_of_heap)) / HEAP_BLOCK_SIZE;
}

void mark_heap_blocks_free(struct heap* heap, int start_block) {
    struct heap_table* table = heap->table;

    for (int i = start_block; i < (int) table->total_entries_num; i++) {
        HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];
        table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
        if (!(entry & HEAP_BLOCK_HAS_NEXT)) {
            break; // reach the end of allocation
        }
    }
}