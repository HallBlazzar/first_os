#include "disk_stream.h"
#include "memory/heap/kheap.h"
#include "config.h"
#include <stdbool.h>

struct disk_stream* create_disk_stream(int disk_id) {
    struct disk* target_disk = get_disk(disk_id);
    if (!target_disk) {
        return 0;
    }

    struct disk_stream* target_stream = kzalloc(sizeof(struct disk_stream));
    target_stream->position = 0;
    target_stream->target_disk = target_disk;
    return target_stream;
}

int set_disk_stream_position(struct disk_stream* stream, int position) {
    stream->position = position;
    return 0;
}

int read_from_disk_stream(struct disk_stream* stream, void* output, int target_total_bytes_to_read) {
    int sector = stream->position / DISK_SECTOR_SIZE;
    int offset = stream->position % DISK_SECTOR_SIZE;
    int current_total_bytes_to_read = target_total_bytes_to_read;

    // if current_offset + total_byte_to_read finally exceeds disk sector size,
    // it means buffer size will be overflow --> buffer size if sector size only
    // and unexpected memory will be accessed --> malicious code
    bool overflow = (offset + target_total_bytes_to_read) >= DISK_SECTOR_SIZE;

    if (overflow) {
        current_total_bytes_to_read -= (offset + target_total_bytes_to_read) - DISK_SECTOR_SIZE;
    }

    char buffer[DISK_SECTOR_SIZE]; // set buffer size

    int result = read_disk_block(stream->target_disk, sector, 1, buffer);
    if (result < 0) {
        goto out;
    }

    // Prevent load more data(overflow) than buffer's capacity
    // Means load data under buffer size a time
    for (int i = 0; i < current_total_bytes_to_read; i++) {
        *(char*)output++ = buffer[offset+i];
    }

    // Adjust stream, and load rest of bytes
    // Recursive call
    stream->position += current_total_bytes_to_read;
    // Read haven't finished, still have bytes not read
    if (overflow) {
        result = read_from_disk_stream(stream, output, target_total_bytes_to_read - current_total_bytes_to_read);
    }

out:
    return result;
}

void close_disk_stream(struct disk_stream* stream) {
    kfree(stream);
}