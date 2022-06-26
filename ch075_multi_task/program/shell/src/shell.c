#include "shell.h"
#include "stdio.h"
#include "stdlib.h"
#include "firstos.h"

int main(int argc, char** argv) {
    print("FirstOS v1.0.0\n");
    while (1) {
        print("> ");
        char buffer[1024];
        firstos_readline(buffer, sizeof(buffer), true);
        print("\n");
        firstos_command_run(buffer);
        print("\n");
    }
    return 0;
}