#include "firstos.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main(int argc, char** argv) {
    printf("%i %s\n", argc, argv[0]);

    // try to access a page which the program cannot access
    char* ptr = (char*) 0x00;
    *ptr = 0x50;

    while(1) {}
    return 0;
}

