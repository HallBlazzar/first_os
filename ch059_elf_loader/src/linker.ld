ENTRY(_start)
OUTPUT_FORMAT(binary)

/*
ALIGN --> ensure each section is aligned for easier access for our codes
*/

SECTIONS
{
    . = 1M;
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