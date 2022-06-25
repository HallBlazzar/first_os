#include "isr80h.h"
#include "idt/idt.h"
#include "misc.h"
#include "io.h"
#include "heap.h"
#include "process.h"

void register_system_calls() {
    register_system_call(SYSTEM_CALL_SUM, system_call_0_sum);
    register_system_call(SYSTEM_CALL_PRINT, system_call_1_print);
    register_system_call(SYSTEM_CALL_GET_KEY, system_call_2_get_key);
    register_system_call(SYSTEM_CALL_PUT_CHAR, system_call_3_putchar);
    register_system_call(SYSTEM_CALL_MALLOC, system_call_4_malloc);
    register_system_call(SYSTEM_CALL_FREE, system_call_5_free);
    register_system_call(SYSTEM_CALL_START_LOAD_PROCESS, system_call_6_start_load_process);
    register_system_call(SYSTEM_CALL_INVOKE_SYSTEM_CALL, system_call_7_invoke_system_call);
    register_system_call(SYSTEM_CALL_GET_PROGRAM_ARGUMENTS, system_call_8_get_program_arguments);
}
