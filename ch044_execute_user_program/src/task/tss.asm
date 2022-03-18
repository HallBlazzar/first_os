section .asm

global load_tss

load_tss:
    push ebp
    mov ebp,esp
    mov ax, [ebp + 8] ; TSS segment
    ltr ax
    pop ebp
    ret