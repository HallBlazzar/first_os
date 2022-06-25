#include "process.h"
#include "task/task.h"
#include "task/process.h"
#include "config.h"
#include "status.h"
#include "string/string.h"


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