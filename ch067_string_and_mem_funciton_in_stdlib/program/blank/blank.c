#include "firstos.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main(int argc, char** argv) {
    printf("AA is %i\n", 9527);

    char words[] = "a~wwwwww~~~bb~ccc~wwwwwwwww~m";
    const char* token = strtok(words, "~");

    while(token) {
        printf("%s\n", token);
        token = strtok(NULL, "~");
    }

    while(1) {}
    return 0;
}

