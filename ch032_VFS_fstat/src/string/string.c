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

int strlen_max_with_terminator_checking(const char* ptr, int max_length, char terminator) {
    int i = 0;

    for (i = 0; i < max_length; i++) {
        if(ptr[i] == '\0' || ptr[i] == terminator)
            break;
    }

    return i;
}

int strcmp(const char* str1, const char* str2, int number_of_bytes) {
    unsigned char u1, u2;
    while(number_of_bytes-- > 0) {
        u1 = (unsigned char)* str1++;
        u2 = (unsigned char)* str2++;

        if (u1 != u2)
            return u1 - u2;

        if (u1 == '\0')
            return 0;
    }

    return 0;
}

int strcmp_case_insensitive(const char* str1, const char* str2, int number_of_bytes) {
    unsigned char u1, u2;

    while(number_of_bytes-- > 0) {
        u1 = (unsigned char)* str1++;
        u2 = (unsigned char)* str2++;

        if (u1 != u2 && tolower(u1) != tolower(u2))
            return u1 - u2;

        if (u1 == '\0')
            return 0;
    }

    return 0;
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

char tolower(char str) {
    if (str >= 65 && str <= 90) {
        str += 32;
    }

    return str;
}