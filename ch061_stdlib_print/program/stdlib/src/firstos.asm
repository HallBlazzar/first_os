[BITS 32]

global print:function

; void print(const char* message)
print:
    push ebp
    mov ebp, esp
    push dword[ebp + 8] ; increase stack pointer by 4 bytes(word)
    mov eax, 1; print system call
    int 0x80
    add esp, 4; restore stack pointer
    pop ebp
    ret