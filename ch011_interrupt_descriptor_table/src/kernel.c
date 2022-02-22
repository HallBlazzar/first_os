#include "kernel.h"
#include <stdint.h>
#include <stddef.h>
#include "idt/idt.h"

//uint16_t is 2 bytes
uint16_t* video_mem = 0;
uint16_t terminal_row_of_final_char = 0;
uint16_t terminal_column_of_final_char = 0;

extern void trigger_divide_by_zero();

void terminal_initialize();
void print(const char* str);
void write_char_on_terminal_to_final_char(char c, char colour);
void put_char_on_terminal_to_specific_position(int x, int y, char c, char colour);
uint16_t create_char(char c, char colour);
size_t strlen(const char* str);

void kernel_main() {
    terminal_initialize();
    print("WWWWWWWWWWWWWWWWW!!!!!!\n!!!!!!!!!");

    // Initialize interrupt descriptor table
    initialize_idt();

    trigger_divide_by_zero();
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
    } else {
        put_char_on_terminal_to_specific_position(
            terminal_column_of_final_char,
            terminal_row_of_final_char, c, colour
        );

        // specify new colum/row position of final char
        terminal_column_of_final_char += 1;
        if (terminal_column_of_final_char >= VGA_WIDTH) {
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

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}
