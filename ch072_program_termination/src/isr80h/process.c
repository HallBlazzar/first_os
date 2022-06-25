#include "process.h"
#include "task/task.h"
#include "task/process.h"
#include "config.h"
#include "status.h"
#include "string/string.h"
#include "kernel.h"


void* system_call_6_start_load_process(struct interrupt_frame* interrupt_frame) {
    void* filename_user_ptr = get_task_stack_item(get_current_task(), 0);
    char filename[MAX_PATH];
    int result = copy_string_from_task(get_current_task(), filename_user_ptr, filename, sizeof(filename));

    if (result < 0) {
        goto out;
    }

    char path[MAX_PATH];
    strcpy(path, "0:/");
    strcpy(path + 3, filename); // 3 bytes for '0', ':', '/'

    struct process* process = 0;
    result = load_and_switch_process(path, &process); // change current process

    if (result < 0) {
        goto out;
    }

    switch_task(process->task); // switch current task to process task, also switch page directory
    return_task(&process->task->registers); // back to lower privilege, and execute process task

out:
    return 0; // will never reach there once task switched. Stack trace will lose when switch to user land
}

void* system_call_7_invoke_system_command(struct interrupt_frame* interrupt_frame) {
    struct command_argument* arguments = convert_virtual_address_to_physical(
        get_current_task(), get_task_stack_item(get_current_task(), 0)
    );
    if (!arguments || strlen(arguments[0].argument) == 0) {
        return ERROR(-INVALID_ARG_ERROR);
    }

    struct command_argument* root_command_argument = &arguments[0];
    const char* program_name = root_command_argument->argument; // root should be command itself, like [blank.elf] arg1 arg2 ...
    char path[MAX_PATH];
    strcpy(path, "0:/"); // attach drive id to start of path
    strcpy_max_length(path + 3, program_name, sizeof(path)); // program name with full path

    struct process* process = 0;

    int result = load_and_switch_process(path, &process);

    if (result < 0) {
        return ERROR(result);
    }

    result = inject_process_arguments(process, root_command_argument);
    if (result < 0) {
        return ERROR(result);
    }

    switch_task(process->task); // set current task
    return_task(&process->task->registers); // switch back to userland

    return 0;
}

void* system_call_8_get_program_arguments(struct interrupt_frame* interrupt_frame) {
    struct process* process = get_current_task()->process;
    struct process_arguments* arguments = convert_virtual_address_to_physical(
        get_current_task(), get_task_stack_item(get_current_task(), 0)
    );

    get_process_arguments(process, &arguments->argc, &arguments->argv);

    return 0;
}