#ifndef FILE_H
#define FILE_H
#include "path_parser.h"

typedef unsigned int FILE_SEEK_MODE;
enum {
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
};

typedef unsigned int FILE_MODE;
enum {
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
    FILE_MODE_INVALID
};

struct disk;
typedef void*(*FS_OPEN_FUNCTION)(struct disk* disk, struct path_part* path, FILE_MODE mode);
typedef int (*FS_RESOLVE_FUNCTION)(struct disk* disk); // Check disk valid or not

struct filesystem {
    // filesystem should return 0 from resolve if the provided disk is using its filesystem
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;

    char name[20]; // filesystem name
};


struct file_descriptor {
    // descriptor index
    int index;
    struct filesystem* filesystem;

    // private data for internal file descriptor
    void* private;

    // disk file descriptor should be used on
    struct disk* disk;
};

void fs_init();
int fopen(const char* filename, const char* mode);
void insert_filesystem(struct filesystem* filesystem);
struct filesystem* resolve_filesystem(struct disk* disk);
#endif