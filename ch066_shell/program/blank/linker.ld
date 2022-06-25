ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

/*
ALIGN --> ensure each section is aligned for easier access for our codes
*/

SECTIONS
{
    . = 0x400000; /* as the PROGRAM_VIRTUAL_ADDRESS shows in the config.h, all program in this OS stats from this virtual address */
    .text : ALIGN(4096)
    {
        *(.text)
    }

    /*
        Assembly code other than kernel.asm will be hosted there.
        It could prevent other asm code breaking alignment generated
        by GCC-compiled C codes. But kernel.asm is different. It should
        always be place in the beginning to ensure it will always be the first
        execute code (enter protect mode) execute among all kernel codes.

        now blank.asm contains .data section, this section shouldn't be
        place prior than .asm, or assembly code in .data section will be
        loaded prior than .asm.
    */
    .asm : ALIGN(4096)
    {
        *(.asm)
    }

    .rodata : ALIGN(4096)
    {
        *(.rodata)
    }

    .data : ALIGN(4096)
    {
        *(.data)
    }

    .bss : ALIGN(4096)
    {
        *(COMMON)
        *(.bss)
    }
}