#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Attributes
// https://wiki.osdev.org/Paging
// PCD, Cache Disabled bit. 1 for page cached, 0 for uncached
#define PAGING_CACHE_DISABLED  0b00010000
// PWT, Write Trough bit. 1 for write through cache enabled, 0 for write back enabled
#define PAGING_WRITE_THROUGH   0b00001000
// U/S, User/Supervisor bit. 1 for open access to all ring level, 0 for supervisor ring level only
#define PAGING_ACCESS_FROM_ALL 0b00000100
// R/W, Read/Write bit. 1 for page is both read/writable, 0 for read only
#define PAGING_IS_WRITABLE     0b00000010
// P, Present bit. 1 for page is currently in physical memory, 0 for not in. If page is not presented and called, page fault raised.
#define PAGING_IS_PRESENT      0b00000001

#define TOTAL_PAGING_ENTRIES_PER_TABLE 1024
#define PAGE_SIZE 4096

struct paging_4gb_chunk {
    uint32_t* directory_entry;
};

struct paging_4gb_chunk* create_4gb_page(uint8_t flags);
void free_4gb_page(struct paging_4gb_chunk* chunk);
void switch_current_page_table_directory(struct paging_4gb_chunk* page_table_directory);
void enable_paging();

int set_page_table_entry(uint32_t* directory, void* virtual_address, uint32_t value);
bool address_aligned_for_paging(void* address);

uint32_t* get_directory_of_paging_4gb_chunk(struct paging_4gb_chunk* chunk);

int map_page(struct paging_4gb_chunk* directory, void* virtual_address, void* start_of_physical_address, void* end_of_physical_address, int flags);
int map_range(struct paging_4gb_chunk* directory, void* virtual_address, void* start_of_physical_address, int total_pages, int flags);
int map_single_page(struct paging_4gb_chunk* directory, void* virtual_address, void* start_of_physical_address, int flags);

void* align_address(void* address);
void* align_paging_to_lower_page(void* address);

uint32_t get_page(uint32_t* directory, void* virtual_address);

#endif