#ifndef IDT_H
#define IDT_H

#include "stdint.h"

struct interrupt_frame;

typedef void*(*ISR80H_SYSTEM_CALL)(struct interrupt_frame* interrupt_frame);

struct idt_desc {
    uint16_t offset_1; // offset bit 0-15
    uint16_t selector; // code segment selector in GDT or LDt
    uint8_t zero; //  unused bits
    uint8_t type_attr; // type attributes(gate type, storage segment, DPL, P)
    uint16_t offset_2; // offset 16-31
} __attribute__((packed)); // disable memory size padding. By default, c will align memory size of a structure's members with maximized and same size. With the attribute, each member will only use minimal memory as their data type.

struct idtr_desc {
    uint16_t limit; // limit
    uint32_t base; // 16-47
} __attribute((packed));

// https://faydoc.tripod.com/cpu/pushad.htm
struct interrupt_frame {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t reserved; // represents "original ESP", but not necessary there
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
}__attribute__((packed));

void initialize_idt();
void enable_interrupts();
void disable_interrupts();
void register_system_call(int command_code, ISR80H_SYSTEM_CALL command);

#endif