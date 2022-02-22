#ifndef DISK_H
#define DISK_H

#include "fs/file.h"

typedef unsigned int DISK_TYPE;

// Real HDD
#define DISK_TYPE_REAL 0

struct disk {
    DISK_TYPE disk_type;
    int sector_size;

    struct filesystem* filesystem;
};

void search_and_initialize_disk();
struct disk* get_disk(int index);
int read_disk_block(struct disk* target_disk, unsigned int lba, int total_num_blocks, void* buffer);

#endif
