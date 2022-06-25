#include "firstos.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main(int argc, char** argv) {
    char* ptr = malloc(20);
    strcpy(ptr, "wwwwwwwww~~~~~");

    printf("%s", ptr);

    free(ptr);

    ptr[0] = 'A';
    printf("done");

    while(1) {}
    return 0;
}

