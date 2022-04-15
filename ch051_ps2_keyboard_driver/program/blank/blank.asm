[BITS 32]

section .asm ;; all codes are under .asm section after compiling

global _start

_start:
    push message
    mov eax, 1 ; print system call
    int 0x80
    add esp, 4

    jmp $

section .data
message: db "First Program~~~~~", 0 ; 0 for termination
