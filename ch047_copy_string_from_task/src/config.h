#ifndef CONFIG_H
#define CONFIG_H

#define TOTAL_INTERRUPTS 0x200 // totally 512 interrupts
#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10

// 100MB heap size
#define HEAP_SIZE_BYTES 104857600
#define HEAP_BLOCK_SIZE 4096
#define HEAP_ADDRESS 0x01000000 // check https://wiki.osdev.org/Memory_Map_(x86) . Available size here is 1GB.
#define HEAP_TABLE_ADDRESS 0x00007E00 // check https://wiki.osdev.org/Memory_Map_(x86) . Available size here is 480.5 KB, which is enough for placing 25600 byte table

#define DISK_SECTOR_SIZE 512

#define MAX_FILESYSTEMS 12
#define MAX_FILE_DESCRIPTORS 512

#define MAX_PATH 108

#define TOTAL_GDT_SEGMENTS 6

#define PROGRAM_VIRTUAL_ADDRESS 0x400000
#define USER_PROGRAM_STACK_SIZE 1024 * 16 // must be 4096(4K) aligned --> for paging memory structure
#define START_ADDRESS_OF_PROGRAM_VIRTUAL_STACK_ADDRESS 0x3FF000
#define END_ADDRESS_OF_PROGRAM_VIRTUAL_STACK_ADDRESS START_ADDRESS_OF_PROGRAM_VIRTUAL_STACK_ADDRESS - USER_PROGRAM_STACK_SIZE // Under intel, stack grows DOWN in memory space

#define MAX_MEMORY_ALLOCATION 1024
#define MAX_PROCESSES 32

// offset based on gdt_real
#define USER_DATA_SEGMENT 0x23
#define USER_CODE_SEGMENT 0x1b

#define MAX_SYSTEM_CALLS 1024

#endif