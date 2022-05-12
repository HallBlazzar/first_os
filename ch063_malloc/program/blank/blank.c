#include "firstos.h"
#include "stdlib.h"

int main(int argc, char** argv) {
    print("First programmm\n");

    void* ptr = malloc(512);

    if (ptr) {
        print("Memory allocated\n");
        free(ptr);
        print("Memory freed\n");
    }

    while(1) {
        if (get_key() != 0) {
            print("Key pressed\n");
        }
    }
    return 0;
}

