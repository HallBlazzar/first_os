#include "fat16.h"
#include "status.h"
#include "string/string.h"
#include "disk/disk.h"
#include "disk/disk_stream.h"
#include "memory/memory.h"
#include <stdint.h>
#include "memory/heap/kheap.h"

#define FAT16_SIGNATURE 0x29
#define FAT16_FAT_ENTRY_SIZE 0x02
#define FAT16_BAD_SECTOR 0xFF7
#define FAT16_UNUSED 0x00

// FAT file type
typedef unsigned int FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1

// FAT directory entry attributes bitmask
#define FAT_FILE_READ_ONLY 0x01
#define FAT_FILE_HIDDEN 0x02
#define FAT_FILE_SYSTEM 0X04
#define FAT_FILE_VOLUME_LABEL 0x08
#define FAT_FILE_SUBDIRECTORY 0x10
#define FAT_FILE_ARCHIVED 0x20
#define FAT_FILE_DEVICE 0x40
#define FAT_FILE_RESERVED 0x80


// represent FAT header information in boot.asm
struct extended_fat_header {
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

struct primary_fat_header {
    uint8_t short_jmp_instruction[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t num_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t sectors_big;
} __attribute__((packed));

struct fat_header {
    struct primary_fat_header primary_fat_header;
    // extended header is optional, so make this field union
    union extended_fat_header_union {
        struct extended_fat_header extended_header;
    } shared;
};

// represents "file"
struct fat_directory_item {
    uint8_t filename[8];
    uint8_t extension[3];
    uint8_t attribute; // FAT directory entry attributes bitmask
    uint8_t reserved;
    uint8_t creation_time_tenth_of_a_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_of_first_cluster;
    uint16_t last_modified_time;
    uint16_t last_modified_date;
    uint16_t low_16_bits_of_first_cluster;
    uint32_t filesize;
} __attribute((packed));

// represents "directory"
struct fat_directory {
    struct fat_directory_item* item;
    int total_num_of_items;
    int start_sector_position;
    int end_sector_position;
};

// represents fat entry. can be file or directory
struct fat_item {
    union {
        struct fat_directory_item* item;
        struct fat_directory* directory;
    };

    FAT_ITEM_TYPE item_type;
};

struct fat_private_data {
    struct fat_header header;
    struct fat_directory root_directory;

    // used to stream data clusters
    struct disk_stream* cluster_read_stream;
    // used to stream file allocation table
    struct disk_stream* fat_read_stream;

    // used to stream directory
    struct disk_stream* directory_stream;
};

// represents opened file
struct fat_item_descriptor {
    struct fat_item* item;
    uint32_t  position; // current position of file pointer
};

int resolve_fat16_filesystem(struct disk* disk);
void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode);

static void initialize_fat16_private_data(struct disk* disk, struct fat_private_data* private_data);

int get_fat16_root_directory(struct disk* disk, struct fat_private_data* fat_private, struct fat_directory* directory);
int get_total_items_under_fat16_directory(struct disk* disk, uint32_t directory_start_sector);
int convert_sector_to_absolute_byte_for_fat16(struct disk* disk, int sector);

struct filesystem fat16 = {
    .resolve = resolve_fat16_filesystem,
    .open = fat16_open
};

struct filesystem* initialize_fat16_filesystem() {
    strcpy(fat16.name, "FAT16");
    return &fat16;
}

int resolve_fat16_filesystem(struct disk* disk) {
    int result = 0;
    struct fat_private_data* private_data = kzalloc(sizeof(struct fat_private_data));
    initialize_fat16_private_data(disk, private_data);

    disk->filesystem_private_data = private_data;
    disk->filesystem = &fat16;

    struct disk_stream* stream = create_disk_stream(disk->id);
    if (!stream) {
        result = -NO_FREE_MEM_ERROR;
        goto out;
    }

    if (read_from_disk_stream(stream, &private_data->header, sizeof(private_data->header)) != ALL_OK) {
        result = -IO_ERROR;
        goto out;
    }

    if (private_data->header.shared.extended_header.signature != 0x29) {
        result = -INVALID_FS_SIGNATURE_ERROR;
        goto out;
    }

    if (get_fat16_root_directory(disk, private_data, &private_data->root_directory) != ALL_OK) {
        result = -IO_ERROR;
        goto out;
    }


out:
    if (stream) {
        close_disk_stream(stream);
    }

    if (result < 0) {
        kfree(private_data);
        disk->filesystem_private_data = 0;
    }
    return result;
}

static void initialize_fat16_private_data(struct disk* disk, struct fat_private_data* private_data) {
    memset(private_data, 0, sizeof(struct fat_private_data));
    private_data->cluster_read_stream = create_disk_stream(disk->id);
    private_data->fat_read_stream = create_disk_stream(disk->id);
    private_data->directory_stream = create_disk_stream(disk->id);
}

int get_fat16_root_directory(struct disk* disk, struct fat_private_data* private_data, struct fat_directory* directory) {
    int result = 0;
    struct primary_fat_header* primary_header = &private_data->header.primary_fat_header;
    int root_directory_sector_position = (primary_header->fat_copies * primary_header->sectors_per_fat) + primary_header->reserved_sectors;
    int root_directory_entries = private_data->header.primary_fat_header.root_dir_entries;
    int root_directory_size = (root_directory_entries * sizeof(struct fat_directory_item));
    int total_sectors = root_directory_size / disk->sector_size;
    if (root_directory_size % disk->sector_size) {
        total_sectors += 1; // load 1 more sector
    }

    int total_items = get_total_items_under_fat16_directory(disk, root_directory_sector_position);

    struct fat_directory_item* root_directory = kzalloc(root_directory_size);
    if (!root_directory) {
        result = -NO_FREE_MEM_ERROR;
        goto out;
    }

    struct disk_stream* stream = private_data->directory_stream;
    if (set_disk_stream_position(stream, convert_sector_to_absolute_byte_for_fat16(disk, root_directory_sector_position)) != ALL_OK) {
        result = -IO_ERROR;
        goto out;
    }

    if (read_from_disk_stream(stream, root_directory, root_directory_size) != ALL_OK) {
        result = -IO_ERROR;
        goto out;
    }

    directory->item = root_directory;
    directory->total_num_of_items = total_items;
    directory->start_sector_position = root_directory_sector_position;
    directory->end_sector_position = root_directory_sector_position + (root_directory_size / disk->sector_size);
out:
    return result;
}

int get_total_items_under_fat16_directory(struct disk* disk, uint32_t directory_start_sector) {
    struct fat_directory_item item;
    struct fat_directory_item empty_item;

    memset(&empty_item, 0, sizeof(empty_item));

    struct fat_private_data* private_data = disk->filesystem_private_data;

    int result = 0;
    int i = 0;
    int directory_start_position = directory_start_sector * disk->sector_size;
    struct disk_stream* stream = private_data->directory_stream;

    if (set_disk_stream_position(stream, directory_start_position) != ALL_OK) {
        result = -IO_ERROR;
        goto out;
    }

    while (1) {
        if (read_from_disk_stream(stream, &item, sizeof(item)) != ALL_OK) {
            result = -IO_ERROR;
            goto out;
        }

        if (item.filename[0] == 0x00) { // blank record
            break;
        }

        if (item.filename[0] == 0xE5) { // item unused(free entry)
            continue;
        }

        i++;
    }

    result = i;
out:
    return result;
}

int convert_sector_to_absolute_byte_for_fat16(struct disk* disk, int sector) {
    return sector * disk->sector_size;
}

void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode) {
    return 0;
}