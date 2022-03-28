[BITS 32]

section .asm ;; all codes are under .asm section after compiling

global _start

_start:
    mov eax, 0
    int 0x80

    jmp $
