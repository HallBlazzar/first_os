#include "string.h"

int strlen(const char* ptr) {
    int i = 0;

    while (*ptr != 0) {
        i++;
        ptr += 1;
    }

    return i;
}

int strlen_max(const char* ptr, int max_length) {
    int i = 0;

    for (i = 0; i < max_length; i++) {
        if(ptr[i] == 0)
            break;
    }

    return i;
}

char* strcpy(char* dest, const char* src) {
    char* result = dest;

    while (*src != 0) {
        *dest = *src;
        src += 1;
        dest += 1;
    }

    *dest = 0x00; // end of string

    return result;
}

bool is_digit(char c) {
    return c >= 48 && c <=57;
}

int to_numeric_digit(char c) {
    // char "0"= 0x30 = 48
    return c - 48;
}