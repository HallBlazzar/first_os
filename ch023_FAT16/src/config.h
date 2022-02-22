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

#endif