#ifndef PATH_PARSER_H
#define PATH_PARSER_H

struct path_root {
    int drive_num;
    struct path_part* first;
};

struct path_part {
    const char* part;
    struct path_part* next;
};

struct path_root* parse_path_string_to_path_part(const char* path, const char* current_directory_path);

#endif