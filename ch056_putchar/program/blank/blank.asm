[BITS 32]

section .asm ;; all codes are under .asm section after compiling

global _start

_get_key_loop:
    call get_key
    push eax ; got key
    mov eax, 3 ; putchar
    int 0x80
    add esp, 4

    jmp _get_key_loop

get_key:
    mov eax, 2; get key system call
    int 0x80

    ; if no key read, loop get_key
    cmp eax, 0x00
    je get_key
    
    ret

section .data
message: db "First Program~~~~~", 0 ; 0 for termination
