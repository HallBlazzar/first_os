#include "fat16.h"
#include "status.h"
#include "config.h"
#include "string/string.h"
#include "disk/disk.h"
#include "disk/disk_stream.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "kernel.h"
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
struct fat_file_descriptor {
    struct fat_item* item;
    uint32_t  position; // current position of file pointer
};

int resolve_fat16_filesystem(struct disk* disk);
void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode);

static void initialize_fat16_private_data(struct disk* disk, struct fat_private_data* private_data);

int get_fat16_root_directory(struct disk* disk, struct fat_private_data* fat_private, struct fat_directory* directory);
int get_total_items_under_fat16_directory(struct disk* disk, uint32_t directory_start_sector);
int convert_sector_to_absolute_byte_for_fat16(struct disk* disk, int sector);

struct fat_item* get_directory_entry(struct disk* disk, struct path_part* path);
struct fat_item* find_item_in_directory(struct disk* disk, struct fat_directory* directory, const char* name);
void get_full_relative_filename(struct fat_directory_item* item, char* out, int max_len);
void remove_spaces(char** out, const char* in);
struct fat_item* new_fat_item_for_directory_item(struct disk* disk, struct fat_directory_item* item);
struct fat_directory* load_fat_directory(struct disk* disk, struct fat_directory_item* directory_item);
static uint32_t get_first_cluster(struct fat_directory_item* directory_item);
static int convert_cluster_to_sector(struct fat_private_data* private_data, int cluster);
static int read_data_from_cluster(struct disk* disk, int starting_cluster, int offset, int total_bytes_to_read, void* out);
static int read_data_from_cluster_via_disk_stream(struct disk* disk, struct disk_stream* stream, int cluster, int offset, int total_bytes_to_read, void* out);
static int get_cluster_based_on_offset(struct disk* disk, int starting_cluster, int offset);
static int get_fat_entry(struct disk* disk, int cluster);
static uint32_t get_first_fat_sector(struct fat_private_data* private_data);
void free_directory(struct fat_directory* directory);
struct fat_directory_item* clone_directory_item(struct fat_directory_item* directory_item, int size);
void free_fat_item(struct fat_item* item);

int fat16_read(struct disk* disk, void* descriptor, uint32_t num_of_bytes, uint32_t num_of_blocks, char* out_ptr);

int fat16_seek(void* private_descriptor, uint32_t offset, FILE_SEEK_MODE seek_mode);

struct filesystem fat16 = {
    .resolve = resolve_fat16_filesystem,
    .open = fat16_open,
    .read = fat16_read,
    .seek = fat16_seek,
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
    if (mode != FILE_MODE_READ) {
        return ERROR(-READ_ONLY_ERROR);
    }

    struct fat_file_descriptor* descriptor = 0;
    descriptor = kzalloc(sizeof(struct fat_file_descriptor));
    if (!descriptor) {
        return ERROR(-NO_FREE_MEM_ERROR);
    }

    // find file in given path
    descriptor->item = get_directory_entry(disk, path);
    if (!descriptor->item) {
        return ERROR(-IO_ERROR);
    }

    // beginning of a file
    descriptor->position = 0;

    return descriptor;
}

struct fat_item* get_directory_entry(struct disk* disk, struct path_part* path) {
    struct fat_private_data* private_data = disk->filesystem_private_data;
    struct fat_item* current_item = 0;
    struct fat_item* root_item = find_item_in_directory(disk, &private_data->root_directory, path->part);

    if (!root_item) {
        goto out;
    }

    struct path_part* next_part = path->next;
    current_item = root_item;

    while(next_part != 0) {
        if (current_item->item_type != FAT_ITEM_TYPE_DIRECTORY) {
            current_item = 0;
            break;
        }
        struct fat_item* temp_item = find_item_in_directory(disk, current_item->directory, next_part->part);
        free_fat_item(current_item);
        current_item = temp_item;
        next_part = next_part->next;
    }
out:
    return current_item;
}

struct fat_item* find_item_in_directory(struct disk* disk, struct fat_directory* directory, const char* target_filename) {
    struct fat_item* item = 0;
    char temp_filename[MAX_PATH];

    for (int i = 0; i < directory->total_num_of_items; i++) {
        get_full_relative_filename(&directory->item[i], temp_filename, sizeof(temp_filename));
        // check given file name matches item name
        if (strcmp_case_insensitive(temp_filename, target_filename, sizeof(temp_filename)) == 0) {
            item = new_fat_item_for_directory_item(disk, &directory->item[i]);
        }
    }

    return item;
}

// get file name includes extension
void get_full_relative_filename(struct fat_directory_item* item, char* out, int max_len) {
    memset(out, 0x00, max_len);
    char *temp_out = out;
    remove_spaces(&temp_out, (const char*) item->filename);
    if (item->extension[0] != 0x00 && item->extension[0] != 0x20) {
        *temp_out++ = '.';
        remove_spaces(&temp_out, (const char*) item->extension);
    }
}

// file name will end with space(0x00) if file name length shorter than maximum length,
// or fat16 terminator char(0x20). We need to remove redundant spaces
void remove_spaces(char** out, const char* in) {
    while (*in != 0x00 && *in != 0x20) {
        **out = *in;
        *out += 1;
        in += 1;
    }

    if (*in == 0x20) {
        **out = 0x00;
    }
}

struct fat_item* new_fat_item_for_directory_item(struct disk* disk, struct fat_directory_item* directory_item) {
    struct fat_item* new_item = kzalloc(sizeof(struct fat_item));
    if (!new_item) {
        return 0;
    }

    if (directory_item->attribute & FAT_FILE_SUBDIRECTORY) {
        new_item->directory = load_fat_directory(disk, directory_item);
        new_item->item_type = FAT_ITEM_TYPE_DIRECTORY;
    }

    new_item->item_type = FAT_ITEM_TYPE_FILE;
    // the original directory item might be free anytime
    new_item->item = clone_directory_item(directory_item, sizeof(struct fat_directory_item));

    return new_item;
}

struct fat_directory* load_fat_directory(struct disk* disk, struct fat_directory_item* directory_item) {
    int result = 0;

    struct fat_directory* directory = 0;
    struct fat_private_data* private_data = disk->filesystem_private_data;
    if (!(directory_item->attribute & FAT_FILE_SUBDIRECTORY)) {
        result = -INVALID_ARG_ERROR;
        goto out;
    }

    directory = kzalloc(sizeof(struct fat_directory));
    if (!directory) {
        result = -NO_FREE_MEM_ERROR;
        goto out;
    }

    int cluster = get_first_cluster(directory_item);
    int cluster_sector = convert_cluster_to_sector(private_data, cluster);
    int total_num_of_items = get_total_items_under_fat16_directory(disk, cluster_sector);
    directory->total_num_of_items = total_num_of_items;
    int directory_size = directory->total_num_of_items * sizeof(struct fat_directory_item);

    directory->item = kzalloc(directory_size);
    if (!directory->item) {
        result = -NO_FREE_MEM_ERROR;
        goto out;
    }

    result = read_data_from_cluster(disk, cluster, 0x00, directory_size, directory->item);
    if (result != ALL_OK) {
        goto out;
    }
out:
    if (result != ALL_OK) {
        free_directory(directory);
    }
    return directory;
}

static uint32_t get_first_cluster(struct fat_directory_item* directory_item) {
    return (directory_item->high_16_bits_of_first_cluster) | directory_item->low_16_bits_of_first_cluster;
}

static int convert_cluster_to_sector(struct fat_private_data* private_data, int cluster) {
    return private_data->root_directory.end_sector_position + ((cluster - 2) * private_data->header.primary_fat_header.sectors_per_cluster);
}

static int read_data_from_cluster(struct disk* disk, int starting_cluster, int offset, int total_bytes_to_read, void* out) {
    struct fat_private_data* private_data = disk->filesystem_private_data;
    struct disk_stream* stream = private_data->cluster_read_stream;
    return read_data_from_cluster_via_disk_stream(disk, stream, starting_cluster, offset, total_bytes_to_read, out);
}

static int read_data_from_cluster_via_disk_stream(struct disk* disk, struct disk_stream* stream, int cluster, int offset, int total_bytes_to_read, void* out) {
    int result = 0;
    struct fat_private_data* private_data = disk->filesystem_private_data;
    int size_of_cluster_in_bytes = private_data->header.primary_fat_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_read = get_cluster_based_on_offset(disk, cluster, offset);
    if (cluster_to_read < 0) {
        result = cluster_to_read;
        goto out;
    }

    int offset_from_cluster = offset % size_of_cluster_in_bytes;

    int starting_sector = convert_cluster_to_sector(private_data, cluster_to_read);
    int starting_byte_position = (starting_sector * disk->sector_size) + offset_from_cluster;
    // only size_of_cluster_in_bytes can be read a time at most
    int current_total_bytes_to_read = total_bytes_to_read > size_of_cluster_in_bytes ? size_of_cluster_in_bytes: total_bytes_to_read;

    result = set_disk_stream_position(stream, starting_byte_position);
    if (result != ALL_OK) {
        goto out;
    }

    result = read_from_disk_stream(stream, out, current_total_bytes_to_read);

    if (result != ALL_OK) {
        goto out;
    }

    total_bytes_to_read = total_bytes_to_read - current_total_bytes_to_read;

    if (total_bytes_to_read > 0) {
        result = read_data_from_cluster_via_disk_stream(disk, stream, cluster, offset + current_total_bytes_to_read, total_bytes_to_read, out + current_total_bytes_to_read);
    }
out:
    return result;
}

static int get_cluster_based_on_offset(struct disk* disk, int starting_cluster, int offset) {
    int result = 0;
    struct fat_private_data* private_data = disk->filesystem_private_data;
    int size_of_cluster_in_bytes = private_data->header.primary_fat_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_read = starting_cluster;
    int rest_num_of_cluster = offset / size_of_cluster_in_bytes;
    for (int i = 0; i < rest_num_of_cluster; i++) {
        int entry = get_fat_entry(disk, cluster_to_read);
        if (entry == 0xFF8 || entry == 0xFFF) {
            // last entry in file(0xFFF) or error flag(0xFF8)
            result = -IO_ERROR;
            goto out;
        }

        // bad sector
        if (entry == FAT16_BAD_SECTOR) {
            result = -IO_ERROR;
            goto out;
        }

        // reserved sector
        if (entry == 0xFF0 || entry == 0xFF6) {
            result = -IO_ERROR;
            goto out;
        }

        // unexpected flag, corrupt
        if (entry == 0x00) {
            result = -IO_ERROR;
            goto out;
        }

        cluster_to_read = entry;
    }

    result = cluster_to_read;

out:
    return result;
}

static int get_fat_entry(struct disk* disk, int cluster) {
    int result = -1;
    struct fat_private_data* private_data = disk->filesystem_private_data;
    struct disk_stream* stream = private_data->fat_read_stream;
    if (!stream) {
        goto out;
    }

    uint32_t fat_table_position = get_first_fat_sector(private_data) * disk->sector_size;
    result = set_disk_stream_position(stream, fat_table_position * (cluster * FAT16_FAT_ENTRY_SIZE));

    if (result < 0) {
        goto out;
    }

    uint16_t read_result = 0;
    result = read_from_disk_stream(stream, &read_result, sizeof(read_result));

    if (result < 0) {
        goto out;
    }

    result = read_result;
out:
    return result;
}

static uint32_t get_first_fat_sector(struct fat_private_data* private_data) {
    return private_data->header.primary_fat_header.reserved_sectors;
}

void free_directory(struct fat_directory* directory) {
    if (!directory) {
        return;
    }

    if (!directory -> item) {
        kfree(directory->item);
    }

    kfree(directory);
}

struct fat_directory_item* clone_directory_item(struct fat_directory_item* directory_item, int size) {
    struct fat_directory_item* copied_item = 0;

    if(size < sizeof(struct fat_directory_item)) {
        return 0;
    }

    copied_item = kzalloc(size);

    if(!copied_item) {
        return 0;
    }

    memcpy(copied_item, directory_item, size);

    return copied_item;
}

void free_fat_item(struct fat_item* item) {
    if (item->item_type == FAT_ITEM_TYPE_DIRECTORY) {
        free_directory(item->directory);
    } else if (item->item_type == FAT_ITEM_TYPE_FILE) {
        kfree(item->item);
    } else {
        // should throw kernel panic
    }

    kfree(item);
}

int fat16_read(struct disk* disk, void* descriptor, uint32_t num_of_bytes, uint32_t num_of_blocks, char* out_ptr) {
    int result = 0;

    struct fat_file_descriptor* fat_descriptor = descriptor;
    struct fat_directory_item* directory_item = fat_descriptor->item->item;
    int offset = fat_descriptor->position;

    for (uint32_t i = 0; i < num_of_blocks; i++) {
        result = read_data_from_cluster(disk, get_first_cluster(directory_item), offset, num_of_bytes, out_ptr);
        if (IS_ERROR(result)) {
            goto out;
        }
        out_ptr += num_of_bytes;
        offset += num_of_bytes;
    }

    result = num_of_blocks; // response should be total num of blocks read
out:
    return result;
}

int fat16_seek(void* private_descriptor, uint32_t offset, FILE_SEEK_MODE seek_mode) {
    int result = 0;
    struct fat_file_descriptor* descriptor = private_descriptor;
    struct fat_item* descriptor_item = descriptor->item;

    if (descriptor_item->item_type != FAT_ITEM_TYPE_FILE) {
        result = -INVALID_ARG_ERROR;
        goto out;
    }

    struct fat_directory_item* return_item = descriptor_item->item;
    if (offset >= return_item->filesize) {
        result = -IO_ERROR;
        goto out;
    }

    switch (seek_mode) {
        case SEEK_SET:
            descriptor->position = offset;
            break;
        case SEEK_END:
            result = -UNIMPLEMENTED_ERROR;
            break;

        case SEEK_CURRENT:
            descriptor->position += offset;
            break;

        default:
            result = -INVALID_ARG_ERROR;
            break;
    }
out:

    return result;
}