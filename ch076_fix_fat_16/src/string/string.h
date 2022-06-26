#ifndef STRING_H
#define STRING_H
#include <stdbool.h>

int strlen(const char* ptr);
int strlen_max(const char* ptr, int max_length);
int strlen_max_with_terminator_checking(const char* ptr, int max_length, char terminator);
int strcmp_case_insensitive(const char* str1, const char* str2, int number_of_bytes);
int strcmp(const char* str1, const char* str2, int number_of_bytes);
char* strcpy(char* dest, const char* src);
char* strcpy_max_length(char* dest, const char* src, int max_length);
bool is_digit(char c);
int to_numeric_digit(char c);
char tolower(char str);

#endif