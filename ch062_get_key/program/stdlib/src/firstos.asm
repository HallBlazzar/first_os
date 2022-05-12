[BITS 32]

global print:function
global get_key:function

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

; int get_key()
get_key:
    push ebp
    mov ebp, esp
    mov eax, 2; print system call
    int 0x80
    pop ebp
    ret