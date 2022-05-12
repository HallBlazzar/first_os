#include "firstos.h"

int main(int argc, char** argv) {
    print("First programmm\n");
    while(1) {
        if (get_key() != 0) {
            print("Key pressed\n");
        }
    }
    return 0;
}

