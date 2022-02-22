#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "kernel.h"
#include "io/io.h"

struct idt_desc idt_descriptor_array[TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

extern void idt_load(struct idtr_desc* ptr); // function, idt_load, in idt.asm
extern void int21h();
extern void no_interrupt();

void initialize_idt();
void set_idt(int interrupt_num, void* handler_address);

void idt_zero();

void int21h_handler();
void no_interrupt_handler();

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

