#include "kernel.h"
#include <stdint.h>
#include <stddef.h>
#include "idt/idt.h"
#include "string/string.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "disk/disk.h"
#include "fs/path_parser.h"
#include "disk/disk_stream.h"
#include "fs/file.h"
#include "gdt/gdt.h"
#include "config.h"
#include "memory/memory.h"
#include "task/tss.h"
#include "task/task.h"
#include "task/process.h"
#include "status.h"
#include "isr80h/isr80h.h"
#include "keyboard/keyboard.h"

//uint16_t is 2 bytes
uint16_t* video_mem = 0;
uint16_t terminal_row_of_final_char = 0;
uint16_t terminal_column_of_final_char = 0;

void terminal_initialize();
void print(const char* str);
void write_char_on_terminal_to_final_char(char c, char colour);
void put_char_on_terminal_to_specific_position(int x, int y, char c, char colour);
uint16_t create_char(char c, char colour);
void backspace_on_terminal();

static struct paging_4gb_chunk* kernel_chunk = 0;

struct tss tss;

struct gdt real_gdt[TOTAL_GDT_SEGMENTS];
struct structured_gdt structured_gdt[TOTAL_GDT_SEGMENTS] = {
        {.base = 0x00, .limit = 0x00, .type = 0x00}, // NULL Segment
        {.base = 0x00, .limit = 0xffffffff, .type = 0x9a}, // Kernel Code Segment
        {.base = 0x00, .limit = 0xffffffff, .type = 0x92}, // Kernel Data Segment
        {.base = 0x00, .limit = 0xffffffff, .type = 0xf8}, // User Code Segment
        {.base = 0x00, .limit = 0xffffffff, .type = 0xf2}, // User Data Segment
        {.base = (uint32_t)&tss, .limit = sizeof(tss), .type = 0xE9}, // TSS Segment
};

void kernel_main() {
    terminal_initialize();

    // Create and load GDT
    memset(real_gdt, 0x00, sizeof(real_gdt));
    convert_structured_gdt_to_gdt(real_gdt, structured_gdt, TOTAL_GDT_SEGMENTS);
    load_gdt(real_gdt, sizeof(real_gdt));

    // Initialize heap
    initialize_kheap();

    // Initialize filesystem
    fs_init();

    // Search/initialize disk(also resolve filesystem)
    search_and_initialize_disk();

    // Initialize interrupt descriptor table
    initialize_idt();

    // Setup TSS
    memset(&tss, 0x00, sizeof(tss));
    tss.esp0 = 0x600000; // kernel stack address should be
    tss.ss0 = KERNEL_DATA_SELECTOR;

    // Load TSS
    load_tss(0x28); // 0x28 is offset in real_gdt

    // Setup paging
    // 1. allocate heap with size TOTAL_PAGING_ENTRIES_PER_TABLE * 4byte(entry size)
    kernel_chunk = create_4gb_page(PAGING_IS_WRITABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);

    // 2. Switch current page table directory to kernel chunk's page table directory. Means load the kernel chunk into memory
    switch_current_page_table_directory(kernel_chunk);

    // Enable paging
    enable_paging();

    register_system_calls();

    initialize_keyboard();

    struct process* process = 0;
    int result = load_and_switch_process("0:/blank.elf", &process);
    if (result != ALL_OK) {
        panic("Failed to load blank.elf\n");
    }

    struct command_argument argument;
    strcpy(argument.argument, "Testttt");
    argument.next = 0x00;

    inject_process_arguments(process, &argument);

    run_first_task();

    panic("System halt ....");
}

void terminal_initialize() {
    // reset terminal content
    video_mem = (uint16_t*)(0xB8000);

    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            put_char_on_terminal_to_specific_position(x, y, ' ', 0);
        }
    }

    // reset final char position on termianl
    terminal_column_of_final_char = 0;
    terminal_row_of_final_char = 0;
}

void print(const char* str) {
    size_t len = strlen(str);
    for (int i = 0; i < len; i++) {
        write_char_on_terminal_to_final_char(str[i], 15);
    }
}

void write_char_on_terminal_to_final_char(char c, char colour) {
    if (c == '\n') {
        terminal_row_of_final_char += 1;
        terminal_column_of_final_char = 0;
    } else if (c == 0x08) { // backspace
        backspace_on_terminal();
    }
    else {
        put_char_on_terminal_to_specific_position(
            terminal_column_of_final_char,
            terminal_row_of_final_char, c, colour
        );

        // specify new colum/row position of final char
        terminal_column_of_final_char += 1;
        if (terminal_column_of_final_char > VGA_WIDTH) {
            terminal_column_of_final_char = 0;
            terminal_row_of_final_char += 1;
        }
    }
}

void put_char_on_terminal_to_specific_position(int x, int y, char c, char colour) {
    video_mem[(y * VGA_WIDTH) + x] = create_char(c, colour);
}

uint16_t create_char(char c, char colour) {
    // left move colour for 1 byte(8 bit)
    // and put character with "or" operator
    return (colour << 8) | c;
}

void backspace_on_terminal() {
    if (terminal_row_of_final_char == 0 && terminal_column_of_final_char == 0) {
        return;
    }

    if (terminal_column_of_final_char == 0) {
        terminal_row_of_final_char -= 1;
        terminal_column_of_final_char = VGA_WIDTH;
    }

    terminal_column_of_final_char -= 1;
    write_char_on_terminal_to_final_char(' ', 15);
    terminal_column_of_final_char -= 1;
}

void panic(const char* message) {
    print(message);
    while (1) {}
}

void load_kernel_page() {
    load_kernel_registers();
    switch_current_page_table_directory(kernel_chunk);
}
