#include "elfloader.h"
#include "fs/file.h"
#include "status.h"
#include <stdbool.h>
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "string/string.h"
#include "memory/paging/paging.h"
#include "kernel.h"
#include "config.h"


const char elf_signature[] = {0x7f, 'E', 'L', 'F'};

static bool validate_elf_signature(void* buffer);
static bool validate_elf_class(struct elf_header* header);
static bool validate_elf_encoding(struct elf_header* header);
static bool valid_elf_executable(struct elf_header* header);
static bool valid_elf_contains_program_header(struct elf_header* header);
char* get_elf_string_table(struct elf_header* header);

int load_process_from_elf(struct elf_file* elf_file);
int get_process_elf_pheaders(struct elf_file* elf_file);
int get_single_process_elf_pheader(struct elf_file* elf_file, struct elf32_phdr* pheader);
int load_pt_load_from_process_pheader(struct elf_file* elf_file, struct elf32_phdr* pheader);


static bool validate_elf_signature(void* buffer) {
    return memcmp(buffer, (void*) elf_signature, sizeof(elf_signature)) == 0;
}

static bool validate_elf_class(struct elf_header* header) {
    // only support 32 bit binaries
    return header->e_ident[EI_CLASS] == ELFCLASSNONE || header->e_ident[EI_CLASS] == ELFCLASS32;
}

static bool validate_elf_encoding(struct elf_header* header) {
    return header->e_ident[EI_DATA] == ELFDATANONE || header->e_ident[EI_DATA] == ELFDATA2LSB;
}

static bool valid_elf_executable(struct elf_header* header) {
    // prevent load elf from space lower than virtual address, which unexpected kernel data will be accessed
    return header->e_type == ET_EXEC && header->e_entry >= PROGRAM_VIRTUAL_ADDRESS;
}

static bool valid_elf_contains_program_header(struct elf_header* header) {
    return header->e_phoff != 0;
}

void* get_elf_memory(struct elf_file* file) {
    return file->elf_memory;
}

struct elf_header* get_elf_header(struct elf_file* file) {
    // start of an elf file is header, which will be the first place in the memory
    return file->elf_memory;
}

// get session header
struct elf32_shdr* get_elf_sheader(struct elf_header* header) {
    // address of it is header address + offset
    return (struct elf32_shdr*)((int)header + header->e_shoff);
}

// get program header
struct elf32_phdr* get_elf_pheader(struct elf_header* header) {
    if(header->e_phoff == 0) {
        return 0;
    }

    return (struct elf32_phdr*)((int) header + header->e_phoff);
}

struct elf32_phdr* get_elf_program_header_by_entry(struct elf_header* header, int index) {
    return &get_elf_pheader(header)[index];
}

struct elf32_shdr* get_elf_section_by_entry(struct elf_header* header, int index) {
    return &get_elf_sheader(header)[index];
}

char* get_elf_string_table(struct elf_header* header) {
    return (char*) header + get_elf_section_by_entry(header, header->e_shstrndx)->sh_offset;
}

void* get_elf_physical_address(struct elf_file* file, struct elf32_phdr* phdr) {
    return get_elf_memory(file) + phdr->p_offset;
}

void* get_efl_file_virtual_base(struct elf_file* file) {
    return file->virtual_base_address;
}

void* get_efl_file_virtual_end(struct elf_file* file) {
    return file->virtual_end_address;
}

void* get_efl_file_physical_base(struct elf_file* file) {
    return file->physical_base_address;
}

void* get_efl_file_physical_end(struct elf_file* file) {
    return file->physical_end_address;
}

int validate_elf_loadable(struct elf_header* header) {
    return (validate_elf_signature(header) && validate_elf_class(header) && validate_elf_encoding(header) &&
            valid_elf_contains_program_header(header)) ? ALL_OK : -INVALID_FORMAT_ERROR;
}

int load_elf_file(const char* filename, struct elf_file** loaded_elf) {
    struct elf_file* elf_file = kzalloc(sizeof(struct elf_file));

    int fd = 0;
    int result = fopen(filename, "r");

    if (result <= 0) {
        result = -IO_ERROR;
        goto out;
    }
    fd = result;

    struct file_stat file_state;
    result = fstat(fd, &file_state);

    if (result < 0) {
        goto out;
    }

    elf_file -> elf_memory = kzalloc(file_state.file_size);
    result = fread(elf_file->elf_memory, file_state.file_size, 1, fd);

    if (result < 0) {
        goto out;
    }

    result = load_process_from_elf(elf_file);
    if (result < 0) {
        goto out;
    }

    *loaded_elf = elf_file;

out:
    fclose(fd);
    return result;
}

int load_process_from_elf(struct elf_file* elf_file) {
    int result = 0;
    struct elf_header* header = get_elf_header(elf_file);
    result = validate_elf_loadable(header);

    if (result < 0) {
        goto out;
    }

    result = get_process_elf_pheaders(elf_file);

    if (result < 0) {
        goto out;
    }

out:
    return result;
}

int get_process_elf_pheaders(struct elf_file* elf_file) {
    int result = 0;
    struct elf_header* header = get_elf_header(elf_file);

    for (int i = 0; i < header->e_phnum; i++) {
        struct elf32_phdr* pheader = get_elf_program_header_by_entry(header, i);
        result = get_single_process_elf_pheader(elf_file, pheader);
        if (result < 0) {
            break;
        }
    }

    return result;
}

int get_single_process_elf_pheader(struct elf_file* elf_file, struct elf32_phdr* pheader) {
    int result = 0;

    switch (pheader->p_type) {
        case PT_LOAD:
            result = load_pt_load_from_process_pheader(elf_file, pheader);
        break;
    }

    return result;
}

int load_pt_load_from_process_pheader(struct elf_file* elf_file, struct elf32_phdr* pheader) {
    // elf_file->virtual_base_address >= (void*) pheader->p_vaddr --> reset required
    // elf_file->virtual_base_address == 0x00 --> virtual address unset
    if (elf_file->virtual_base_address >= (void*) pheader->p_vaddr || elf_file->virtual_base_address == 0x00) {
        elf_file->virtual_base_address = (void*) pheader->p_vaddr;
        elf_file->physical_base_address = get_elf_memory(elf_file) + pheader->p_offset;
    }

    unsigned int end_virtual_address = pheader->p_vaddr + pheader->p_filesz;
    if (elf_file->virtual_end_address <= (void*)(end_virtual_address) || elf_file->virtual_end_address == 0x00) {
        elf_file->virtual_end_address = (void*) end_virtual_address;
        elf_file->physical_end_address = get_elf_memory(elf_file) + pheader->p_offset + pheader->p_filesz;
    }

    return 0;
}

void close_elf_file(struct elf_file* file) {
    if (!file) {
        return;
    }
    kfree(file->elf_memory);
    kfree(file);
}