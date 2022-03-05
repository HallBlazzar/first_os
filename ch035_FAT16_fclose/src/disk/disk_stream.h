#ifndef DISK_STREAM_H
#define DISK_STREAM_H

#include "disk.h"

struct disk_stream {
    int position;
    struct disk* target_disk;
};

struct disk_stream* create_disk_stream(int disk_id);
int set_disk_stream_position(struct disk_stream* stream, int position);
int read_from_disk_stream(struct disk_stream* stream, void* output, int target_total_bytes_to_read);
void close_disk_stream(struct disk_stream* stream);

#endif
