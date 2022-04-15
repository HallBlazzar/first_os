#ifndef KERNEL_H
#define KERNEL_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 20

#define MAX_PATH_LENGTH 100

void kernel_main();
void print(const char* str);
void write_char_on_terminal_to_final_char(char c, char colour);

void panic(const char* message);
void load_kernel_page();
void load_kernel_registers();

#define ERROR(value) (void*) (value)
#define INT_ERROR(value) (int)(value)
#define IS_ERROR(value) ((int)value< 0)

#endif