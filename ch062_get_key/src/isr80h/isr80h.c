#include "isr80h.h"
#include "idt/idt.h"
#include "misc.h"
#include "io.h"

void register_system_calls() {
    register_system_call(SYSTEM_CALL_SUM, system_call_0_sum);
    register_system_call(SYSTEM_CALL_PRINT, system_call_1_print);
    register_system_call(SYSTEM_CALL_GET_KEY, system_call_2_get_key);
    register_system_call(SYSTEM_CALL_PUT_CHAR, system_call_3_putchar);
}
