#ifndef GDT_H
#define GDT_H
#include <stdint.h>

// represent GDT segments
// https://wiki.osdev.org/Global_Descriptor_Table
// see Table and Segment Descriptor sections for content of each entry
struct gdt {
    uint16_t segment; // 0-15 bits
    uint16_t base_first; // 0-15 bits
    uint8_t base; // base 16-23 bits (32-39)
    uint8_t access; // access byte(40-47)
    uint8_t high_flags; // high 4 bit for flags(52-55) and low 4bit for limit(48-51). "1111" stands for "F"
    uint8_t base_24_31_bits; // base 24-31 bits(56-63)
}__attribute__((packed));

// represent a structured GDT segment
// based on struct gdt, but more representive
struct structured_gdt {
    uint32_t base;
    uint32_t limit;
    uint8_t type;
};

void load_gdt(struct gdt* gdt, int size); // assembly function
void convert_structured_gdt_to_gdt(struct gdt* gdt, struct structured_gdt* structured_gdt, int total_entries);

#endif
