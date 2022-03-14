#ifndef KERNEL_H
#define KERNEL_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 20

#define MAX_PATH_LENGTH 100

void kernel_main();
void print(const char* str);
void panic(const char* message);

#define ERROR(value) (void*) (value)
#define INT_ERROR(value) (int)(value)
#define IS_ERROR(value) ((int)value< 0)

#endif