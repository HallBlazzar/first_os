#ifndef STRING_H
#define STRING_H
#include <stdbool.h>

int strlen(const char* ptr);
int strlen_max(const char* ptr, int max_length);
char* strcpy(char* dest, const char* src);
bool is_digit(char c);
int to_numeric_digit(char c);

#endif