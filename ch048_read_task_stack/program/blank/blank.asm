[BITS 32]

section .asm ;; all codes are under .asm section after compiling

global _start

_start:
    push 20
    push 30
    mov eax, 0 ; call system call 0, sum
    int 0x80
    add esp, 8 ; restore stack. both push 20 and push 30 consumes 4 bytes

    jmp $
