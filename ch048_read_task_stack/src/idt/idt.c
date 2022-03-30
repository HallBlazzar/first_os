#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "kernel.h"
#include "io/io.h"
#include "task/task.h"

struct idt_desc idt_descriptor_array[TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

ISR80H_SYSTEM_CALL isr80h_system_calls[MAX_SYSTEM_CALLS];

extern void idt_load(struct idtr_desc* ptr); // function, idt_load, in idt.asm
extern void int21h();
extern void no_interrupt();
extern void isr80h_wrapper();

void initialize_idt();
void set_idt(int interrupt_num, void* handler_address);

void idt_zero();

void int21h_handler();
void no_interrupt_handler();

void* handle_system_calls_for_isr80h(int command, struct interrupt_frame* interrupt_frame);

void initialize_idt() {
    memset(idt_descriptor_array, 0, sizeof(idt_descriptor_array));
    idtr_descriptor.limit = sizeof(idt_descriptor_array) - 1;
    idtr_descriptor.base = (uint32_t) idt_descriptor_array;

    // give all interrupt a default handler
    for (int i=0; i<TOTAL_INTERRUPTS; i++) {
        set_idt(i, no_interrupt);
    }

    set_idt(0, idt_zero);
    set_idt(0x21, int21h);
    set_idt(0x80, isr80h_wrapper);

    // load interrupt descriptor table

    idt_load(&idtr_descriptor);
}

void idt_zero() {
    print("Divide by zero error\n");
    outb(0x20, 0x20); // acknowledgement
}

void int21h_handler() {
    print("Keyboard pressed!\n");
    outb(0x20, 0x20); // just ack
}

void no_interrupt_handler() {
    outb(0x20, 0x20); // just ack
}

void set_idt(int interrupt_num, void* handler_address) {
    struct idt_desc* desc = &idt_descriptor_array[interrupt_num];
    desc->offset_1 = (uint32_t) handler_address & 0x0000ffff; // lower 16 bits of handler address
    desc->selector = KERNEL_CODE_SELECTOR;
    desc->zero = 0x00;
    desc->type_attr = 0xEE; //[P DPL 0 Gate Type] https://wiki.osdev.org/Interrupt_Descriptor_Table
    desc->offset_2 = (uint32_t) handler_address >> 16; //higher 16 bits of handler address
}

void* isr80h_handler(int system_call_code, struct interrupt_frame* interrupt_frame) {
    void* result = 0;
    load_kernel_page();
    save_current_task_state(interrupt_frame);

    result = handle_system_calls_for_isr80h(system_call_code, interrupt_frame);
    load_task_page();

    return result;
}

void* handle_system_calls_for_isr80h(int command, struct interrupt_frame* interrupt_frame) {
    void* result = 0;

    if (command < 0 || command >= MAX_SYSTEM_CALLS) {
        // Invalid command
        return 0;
    }

    ISR80H_SYSTEM_CALL system_call_handler = isr80h_system_calls[command];

    // Calling a non-exist command
    if (!system_call_handler) {
        return 0;
    }

    result = system_call_handler(interrupt_frame);

    return result;
}

void register_system_call(int command_code, ISR80H_SYSTEM_CALL command) {
    if (command_code < 0 || command_code >= MAX_SYSTEM_CALLS) {
        // when calling this function, it means processor is currently in kernel land
        // then the system should be halt to prevent desasters
        panic("Command is out of bounds\n");
    }

    if (isr80h_system_calls[command_code]) {
        panic("Try to overwrite an existing command");
    }

    isr80h_system_calls[command_code] = command;
}