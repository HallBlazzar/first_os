#include "file.h"
#include "config.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "status.h"
#include "kernel.h"
#include "disk/disk.h"
#include "string/string.h"

#include "fat/fat16.h"

struct filesystem* filesystems[MAX_FILESYSTEMS];
struct file_descriptor* file_descriptors[MAX_FILE_DESCRIPTORS];

static struct filesystem** get_free_filesystem();

void load_filesystems();
static void statically_load_filesystem();

FILE_MODE get_file_mode_by_string(const char* mode_string);

void insert_filesystem(struct filesystem* filesystem) {
    struct filesystem** fs;
    fs = get_free_filesystem();
    if (!fs) {
        print("Encounter problem while inserting filesystem");
        while (1) {}
    }
    *fs = filesystem; // set value in filesystems[]
}

static struct filesystem** get_free_filesystem() {
    int i = 0;
    for (i = 0; i < MAX_FILESYSTEMS; i++) {
        if (filesystems[i] == 0) {
            return &filesystems[i];
        }
    }
    return 0;
}

void fs_init() {
    memset(file_descriptors, 0, sizeof(file_descriptors));
    load_filesystems();
}

void load_filesystems() {
    memset(filesystems, 0, sizeof(filesystems));
    statically_load_filesystem();
}

static void statically_load_filesystem() {
    insert_filesystem(initialize_fat16_filesystem());
}

int create_new_file_descriptor(struct file_descriptor** target_file_descriptor) {
    int result = -NO_FREE_MEM_ERROR;
    for (int i = 0; i < MAX_FILE_DESCRIPTORS; i++) {
        if (file_descriptors[i] == 0) {
            struct file_descriptor* new_file_descriptor = kzalloc(sizeof(struct file_descriptor));
            // Descriptor starts from 1
            new_file_descriptor -> index = i + 1;
            file_descriptors[i] = new_file_descriptor;
            *target_file_descriptor = new_file_descriptor;
            result = 0;
            break;
        }
    }

    return result;
}

static struct file_descriptor* get_file_descriptor(int file_descriptor_index) {
    if (file_descriptor_index <= 0 || file_descriptor_index >= MAX_FILE_DESCRIPTORS) {
        return 0;
    }

    // Descriptor starts from 1
    int real_index = file_descriptor_index - 1;
    return file_descriptors[real_index];
}

struct filesystem* resolve_filesystem(struct disk* disk) {
    struct filesystem* target_filesystem = 0;
    for (int i = 0; i < MAX_FILESYSTEMS ; i++) {
        if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0) {
            target_filesystem = filesystems[i];
            break;
        }
    }
    return target_filesystem;
}

int fopen(const char* filename, const char* mode_string) {
    int result = 0;

    struct path_root* root_path = parse_path_string_to_path_part(filename, NULL);

    if (!root_path) {
        result = -INVALID_ARG_ERROR;
        goto out;
    }

    // Directly open root path (e.g., 1:/) is invalid
    if (!root_path->first) {
        result = -INVALID_ARG_ERROR;
        goto out;
    }

    // check disk exist
    struct disk* disk = get_disk(root_path->drive_num);
    if (!disk) {
        result = -IO_ERROR;
        goto out;
    }

    // check disk contains/binds filesystem
    if (!disk->filesystem) {
        result = -IO_ERROR;
        goto out;
    }

    FILE_MODE file_mode = get_file_mode_by_string(mode_string);
    if (file_mode == FILE_MODE_INVALID) {
        result = -INVALID_ARG_ERROR;
        goto out;
    }

    void *file_descriptor_private_data = disk->filesystem->open(disk, root_path->first, file_mode);
    if (IS_ERROR(file_descriptor_private_data)) {
        result = INT_ERROR(file_descriptor_private_data);
        goto out;
    }

    struct file_descriptor* descriptor = 0;
    result = create_new_file_descriptor(&descriptor);
    if (result < 0) {
        goto out;
    }
    descriptor->filesystem = disk->filesystem;
    descriptor->private = file_descriptor_private_data;
    descriptor->disk = disk;
    result = descriptor->index;
out:
    // fopen never fail, just return 0 in worst case
    if (result < 0) {
        result = 0;
    }
    return result;
}

FILE_MODE get_file_mode_by_string(const char* mode_string) {
    FILE_MODE file_mode = FILE_MODE_INVALID;

    if (strcmp(mode_string, "r", 1) == 0) {
        file_mode = FILE_MODE_READ;
    } else if (strcmp(mode_string, "w", 1) == 0) {
        file_mode = FILE_MODE_WRITE;
    } else if (strcmp(mode_string, "a", 1) == 0) {
        file_mode = FILE_MODE_APPEND;
    }

    return file_mode;
}