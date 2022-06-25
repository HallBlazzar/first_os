#include "stdlib.h"
#include "firstos.h"

void* malloc(size_t size) {
    return firstos_malloc(size);
}

void free(void* ptr) {
    firstos_free(ptr);
}

// convert given integer as string
char* itoa(int i) {
    static char text[12]; // fix memory address of this variable instead of placing it in stack

    int loc = 11;
    text[11] = 0;

    char neg = 1;
    if (i >= 0) {
        neg = 0;
        i = -i;
    }

    while (i) {
        text[--loc] = '0' - (i % 10);
        i /= 10;
    }

    if (loc == 11)
        text[--loc] = '0';

    if (neg)
        text[--loc] = '-';

    return &text[loc];
}