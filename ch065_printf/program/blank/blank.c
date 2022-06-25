#include "firstos.h"
#include "stdlib.h"
#include "stdio.h"

int main(int argc, char** argv) {
    printf("AA is %i\n", 9527);
    print("First programmm\n");
    print(itoa(65535));
    putchar('Z');
    void* ptr = malloc(512);

    if (ptr) {
        print("Memory allocated\n");
        free(ptr);
        print("Memory freed\n");
    }

    char buffer[1024];
    firstos_readline(buffer, sizeof(buffer), true);

    print(buffer);

    while(1) {

    }
    return 0;
}

