#ifndef ELFLOADER_H
#define ELFLOADER_H

#include <stdint.h>
#include <stddef.h>

#include "elf.h"
#include "config.h"

struct elf_file {
    char filename[MAX_PATH];

    int in_memory_size;

    // physical memory address the elf file loaded
    void* elf_memory;

    // lowest virtual base address of the binary
    void* virtual_base_address;

    // highest(end) virtual address
    void* virtual_end_address;

    // physical base address
    void* physical_base_address;

    // physical end address
    void* physical_end_address;
};

int load_elf_file(const char* filename, struct elf_file** loaded_elf);
void close_elf_file(struct elf_file* file);
int validate_elf_loadable(struct elf_header* header);

void* get_elf_memory(struct elf_file* file);
struct elf_header* get_elf_header(struct elf_file* file);
struct elf32_shdr* get_elf_sheader(struct elf_header* header);
struct elf32_phdr* get_elf_pheader(struct elf_header* header);

struct elf32_phdr* get_elf_program_header_by_entry(struct elf_header* header, int index);
struct elf32_shdr* get_elf_section_by_entry(struct elf_header* header, int index);

void* get_elf_physical_address(struct elf_file* file, struct elf32_phdr* phdr);

void* get_efl_file_virtual_base(struct elf_file* file);
void* get_efl_file_virtual_end(struct elf_file* file);
void* get_efl_file_physical_base(struct elf_file* file);
void* get_efl_file_physical_end(struct elf_file* file);

#endif
