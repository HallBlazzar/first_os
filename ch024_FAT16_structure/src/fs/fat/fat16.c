#include "fat16.h"
#include "status.h"
#include "string/string.h"
#include "disk/disk.h"
#include "disk/disk_stream.h"
#include <stdint.h>

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
    union extended_fat_header {
        struct extended_fat_header extended_fat_header;
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

struct filesystem fat16 = {
    .resolve = resolve_fat16_filesystem,
    .open = fat16_open
};

struct filesystem* initialize_fat16_filesystem() {
    strcpy(fat16.name, "FAT16");
    return &fat16;
}

int resolve_fat16_filesystem(struct disk* disk) {
    return 0;
}

void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode) {
    return 0;
}