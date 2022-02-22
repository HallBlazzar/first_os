#ifndef IDT_H
#define IDT_H

#include "stdint.h"

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

void initialize_idt();
void enable_interrupts();
void disable_interrupts();

#endif