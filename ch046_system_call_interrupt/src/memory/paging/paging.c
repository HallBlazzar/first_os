#include "paging.h"
#include "memory/heap/kheap.h"
#include "status.h"

static uint32_t* current_page_table_directory = 0;

void load_page_table_directory(uint32_t* page_table_directory);

int get_paging_indexes(void* virtual_address, uint32_t* directory_index_out, uint32_t* table_index_out);

struct paging_4gb_chunk* create_4gb_page(uint8_t flags) {
    uint32_t* page_table_directory = kzalloc(sizeof(uint32_t) * TOTAL_PAGING_ENTRIES_PER_TABLE);
    int offset = 0;

    for (int i = 0; i < TOTAL_PAGING_ENTRIES_PER_TABLE; i++) {
        uint32_t* page_table_entry = kzalloc(sizeof(uint32_t) * TOTAL_PAGING_ENTRIES_PER_TABLE);
        for (int j = 0; j < TOTAL_PAGING_ENTRIES_PER_TABLE; j++) {
            page_table_entry[j] = (offset + (j * PAGE_SIZE)) | flags;
        }
        offset += (TOTAL_PAGING_ENTRIES_PER_TABLE * PAGE_SIZE);
        page_table_directory[i] = (uint32_t)page_table_entry | flags | PAGING_IS_WRITABLE; // the page should be writable by default
    }

    struct paging_4gb_chunk* chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk_4gb->directory_entry = page_table_directory;
    return chunk_4gb;
}

void free_4gb_page(struct paging_4gb_chunk* chunk) {
    for (int i = 0; i < 1024; i++) {
        uint32_t entry = chunk->directory_entry[i];
        uint32_t* table = (uint32_t*)(entry & 0xfffff000);
        kfree(table);
    }
    kfree(chunk->directory_entry);
    kfree(chunk);
}

void switch_current_page_table_directory(struct paging_4gb_chunk* page_table_directory) {
    load_page_table_directory(page_table_directory->directory_entry);
    current_page_table_directory = page_table_directory->directory_entry;
}

uint32_t* get_directory_of_paging_4gb_chunk(struct paging_4gb_chunk* chunk) {
    return chunk->directory_entry;
}

int set_page_table_entry(uint32_t* directory, void* virtual_address, uint32_t value) {
    if (!address_aligned_for_paging(virtual_address)) {
        return -INVALID_ARG_ERROR;
    }

    uint32_t directory_index = 0;
    uint32_t table_index = 0;

    int result = get_paging_indexes(virtual_address, &directory_index, &table_index);
    if (result < 0) {
        return result;
    }

    uint32_t entry = directory[directory_index];
    uint32_t *table = (uint32_t*)(entry & 0xfffff000); // f for 4 bits, that means we get first 20 bits, which should be table address
    table[table_index] = value; // set page table entry

    return 0;
}

int get_paging_indexes(void* virtual_address, uint32_t* directory_index, uint32_t* table_index) {
    int result = 0;

    if (!address_aligned_for_paging(virtual_address)) {
        result = -INVALID_ARG_ERROR;
        goto out;
    }

    *directory_index = ((uint32_t)virtual_address / (TOTAL_PAGING_ENTRIES_PER_TABLE * PAGE_SIZE));
    *table_index = ((uint32_t)virtual_address % (TOTAL_PAGING_ENTRIES_PER_TABLE * PAGE_SIZE) / PAGE_SIZE);
out:

    return result;
}

bool address_aligned_for_paging(void* address) {
    return ((uint32_t)address % PAGE_SIZE) == 0;
}

int map_page(struct paging_4gb_chunk* directory, void* virtual_address, void* start_of_physical_address, void* end_of_physical_address, int flags) {
    int result = 0;

    if ((uint32_t)virtual_address % PAGE_SIZE) {
        result = -INVALID_ARG_ERROR;
        goto out;
    }

    if ((uint32_t)start_of_physical_address % PAGE_SIZE) {
        result = -INVALID_ARG_ERROR;
        goto out;
    }

    if ((uint32_t)end_of_physical_address % PAGE_SIZE) {
        result = -INVALID_ARG_ERROR;
        goto out;
    }

    if ((uint32_t)end_of_physical_address < (uint32_t)start_of_physical_address) {
        result = -INVALID_ARG_ERROR;
        goto out;
    }

    uint32_t total_bytes = end_of_physical_address - start_of_physical_address;
    int total_pages = total_bytes / PAGE_SIZE;

    result = map_range(directory, virtual_address, start_of_physical_address, total_pages, flags);
out:
    return result;
}

int map_range(struct paging_4gb_chunk* directory, void* virtual_address, void* start_of_physical_address, int total_pages, int flags) {
    int result = 0;

    for (int i = 0; i < total_pages; i++) {
        result = map_single_page(directory, virtual_address, start_of_physical_address, flags);
        if (result == 0)
            break;
        virtual_address += PAGE_SIZE;
        start_of_physical_address += PAGE_SIZE;
    }

    return result;
}

int map_single_page(struct paging_4gb_chunk* directory, void* virtual_address, void* start_of_physical_address, int flags) {
    if (((unsigned int)virtual_address % PAGE_SIZE) || ((unsigned int) start_of_physical_address % PAGE_SIZE)) {
        return -INVALID_ARG_ERROR;
    }

    return set_page_table_entry(directory->directory_entry, virtual_address, (uint32_t) start_of_physical_address | flags);
}

void* align_address(void* address) {
    if((uint32_t)address % PAGE_SIZE) {
        return (void*)((uint32_t)address + PAGE_SIZE - ((uint32_t)address % PAGE_SIZE));
    }

    return address;
}