#include "gdt.h"
#include "kernel.h"

void encode_gdt_entry(uint8_t* target, struct structured_gdt source);

void convert_structured_gdt_to_gdt(struct gdt* gdt, struct structured_gdt* structured_gdt, int total_entries) {
    for (int i = 0; i < total_entries; i++) {
        encode_gdt_entry((uint8_t*)&gdt[i], *structured_gdt); // convert structured gdt to normal gdt
    }
}

void encode_gdt_entry(uint8_t* target, struct structured_gdt source) {
    if ((source.limit > 65536) && ((source.limit & 0xFFF) != 0xFFF)) {
        panic("encode_gdt_entry: Invalid argument");
    }

    target[6] = 0x40;

    if (source.limit > 65536) {
        source.limit = source.limit >> 12;
        target[6] = 0xC0;
    }

    // Encode the limit
    target[0] = source.limit & 0xFF;
    target[1] = (source.limit >> 8) & 0xFF;
    target[6] |= (source.limit >> 16) & 0x0F;

    // Encode the base
    target[2] = source.base & 0xFF;
    target[3] = (source.base >> 8) & 0xFF;
    target[4] = (source.base >> 16) & 0xFF;
    target[7] = (source.base >> 24) & 0xFF;

    // Set type
    target[5] = source.type;
}