#include "file.h"
#include "config.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "status.h"
#include "kernel.h"

struct filesystem* filesystems[MAX_FILESYSTEMS];
struct file_descriptor* file_descriptors[MAX_FILE_DESCRIPTORS];

static struct filesystem** get_free_filesystem();

void load_filesystems();
static void statically_load_filesystem();

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
    struct filesystem* target_filesystem;
    for (int i = 0; i < MAX_FILESYSTEMS ; i++) {
        if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0) {
            target_filesystem = filesystems[i];
            break;
        }
    }
    return target_filesystem;
}

int fopen(const char* filename, const char* mode) {
    return -IO_ERROR;
}