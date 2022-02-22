#include "disk.h"
#include "io/io.h"
#include "memory/memory.h"
#include "config.h"
#include "status.h"

struct disk disk;

int read_sector_from_disk(int lba, int total_num_blocks, void* buffer);

void search_and_initialize_disk() {
    memset(&disk, 0, sizeof(disk));
    disk.disk_type = DISK_TYPE_REAL;
    disk.sector_size = DISK_SECTOR_SIZE;
}

struct disk* get_disk(int index) {
    if (index != 0)
        return 0;

    return &disk;
}

int read_disk_block(struct disk* target_disk, unsigned int lba, int total_num_blocks, void* buffer) {
    if (target_disk != &disk) {
        return -IO_ERROR;
    }

    return read_sector_from_disk(lba, total_num_blocks, buffer);
}

// implement ata_lba_read in boot.asm in C lang
int read_sector_from_disk(int lba, int total_num_blocks, void* buffer) {
    outb(0x1F6, (lba>> 24) | 0xE0);
    outb(0x1F2, total_num_blocks);
    outb(0x1F3, (unsigned char)(lba & 0xff));
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));
    outb(0x1F7, 0x20);

    unsigned short* ptr = (unsigned short*) buffer;

    for (int i = 0; i < total_num_blocks; ++i) {
        // Wait for the disk buffer to be ready
        // Means wait until read 0x08 from 0x1F7 port
        char c = insb(0x1F7);
        while (!(c & 0x08)) { // check until read flag set as 0x08
            c = insb(0x1F7); // wait
        }

        // Copy from HDD to memory
        for (int j = 0; j < 256; ++j) {
            *ptr = insw(0x1F0); // Read 2 bytes (1 word) a time
            ptr++;
        }
    }


    return 0;
}
