#include "firstos.h"

extern int main(int argc, char** argv);

void c_start() {
    struct process_arguments arguments;
    firstos_get_process_arguments(&arguments);

    int result = main(arguments.argc, arguments.argv);

    if (result == 0) {

    }
}