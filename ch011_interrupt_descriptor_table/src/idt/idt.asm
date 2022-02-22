section .asm

global idt_load

idt_load: ; load inturrpt descriptor table
    push ebp
    mov ebp, esp

    mov ebx, [ebp + 8] ; ebp + 8 points to address of fist argument of the funciton
    lidt [ebx] ; load idt

    pop ebp
    ret
