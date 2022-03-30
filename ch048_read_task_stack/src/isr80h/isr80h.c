#include "isr80h.h"
#include "idt/idt.h"
#include "misc.h"

void register_system_calls() {
    register_system_call(SYSTEM_CALL_SUM, system_call_0_sum);
}
