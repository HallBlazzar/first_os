#include "fat16.h"
#include "status.h"
#include "string/string.h"

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