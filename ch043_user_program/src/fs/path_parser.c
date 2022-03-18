#include "path_parser.h"
#include "kernel.h"
#include "string/string.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "status.h"

static int validate_path_format(const char* filename);
static int get_drive_by_path(const char** path);
static struct path_root* create_root(int drive_num);
static const char* get_path_part(const char** path);
struct path_part* parse_path_part(struct path_part* last_part, const char** path);
void free_path(struct path_root* root);


struct path_root* parse_path_string_to_path_part(const char* path, const char* current_directory_path) {
    int result = 0;
    const char* temp_path = path;
    struct path_root* new_path_root = 0;

    if (strlen(path) > MAX_PATH_LENGTH) {
        goto out;
    }

    result = get_drive_by_path(&temp_path);

    if (result < 0) {
        goto out;
    }

    new_path_root = create_root(result);
    if (!new_path_root) {
        goto out;
    }

    struct path_part* first_part = parse_path_part(NULL, &temp_path);
    if (!first_part) {
        goto out;
    }

    new_path_root -> first = first_part;
    struct path_part* next_path_part = parse_path_part(first_part, &temp_path);

    while(next_path_part) {
        next_path_part = parse_path_part(next_path_part, &temp_path);
    }
    out:
    return new_path_root;
}

static int get_drive_by_path(const char** path) {
    if (!validate_path_format(*path)) {
        return -BAD_PATH_ERROR;
    }

    int drive_num = to_numeric_digit(*path[0]);

    // remove driver number prefix from full path
    // totally 3 byte(3 chars) -> ${digit}/:
    // turn the path from
    //     ${digit}:/a/b/c
    // to
    //     /a/b/c
    *path += 3;
    return drive_num;
}

static int validate_path_format(const char* filename) {
    // acceptable path formation
    // ${digit}:/a/b/c
    // ${digit} should be 0-9, stands disk drive number
    int len = strlen_max(filename, MAX_PATH_LENGTH);

    // check
    // 1. path size > 3 (at least x:/)
    // 2. first character is digit(disk num)
    // 3. second and third character is :/
    return (
        len >= 3 &&
        is_digit(filename[0]) && 
        memcmp((void*)&filename[1],":/", 2) == 0
    );
}



static struct path_root* create_root(int drive_num) {
    struct path_root* new_path_root = kzalloc(sizeof(struct path_root));
    new_path_root->drive_num = drive_num;
    new_path_root->first = 0;
    return new_path_root;
}

struct path_part* parse_path_part(struct path_part* last_part, const char** path) {
    const char* path_part_string = get_path_part(path);
    if (!path_part_string) {
        return 0;
    }

    struct path_part* new_path_part = kzalloc(sizeof(struct path_part));
    new_path_part->part = path_part_string;
    new_path_part->next = 0x00;

    if (last_part) {
        last_part->next = new_path_part;
    }

    return new_path_part;
}

static const char* get_path_part(const char** path) {
    char* result_path_part = kzalloc(MAX_PATH_LENGTH);

    // load single path part, character by character
    // if encountering "/"(divider) or "0x00"(end of string), then stop loading
    int i = 0;
    while (**path != '/' && **path != 0x00) {
        result_path_part[i] = **path;
        *path += 1;
        i ++;
    }

    // move to next path part
    if (**path == '/') {
        *path += 1;
    }

    if (i == 0) {
        kfree(result_path_part);
        result_path_part = 0;
    }

    return result_path_part;
}

void free_path(struct path_root* root) {
    struct path_part* current_path_part = root->first;

    while(current_path_part) {
        struct path_part* next_path_part = current_path_part->next;
        kfree((void*) current_path_part->part);
        current_path_part = next_path_part;
    }

    kfree(root);
}

