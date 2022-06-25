#include "stdlib.h"
#include "firstos.h"

void* malloc(size_t size) {
    return firstos_malloc(size);
}

void free(void* ptr) {
    firstos_free(ptr);
}