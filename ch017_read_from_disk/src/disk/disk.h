#ifndef DISK_H
#define DISK_H

int read_sector_from_disk(int lba, int total_num_blocks, void* buffer);

#endif
