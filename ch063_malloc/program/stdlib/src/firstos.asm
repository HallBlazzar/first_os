[BITS 32]

section .asm

global print:function
global get_key:function
global firstos_malloc:function
global firstos_free:function

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
    mov eax, 2; get key system call. get keyboard key
    int 0x80
    pop ebp
    ret

; void* firstos_malloc(size_t size)
firstos_malloc:
    push ebp
    mov ebp, esp
    mov eax, 4; malloc system call. allocat memory to process
    push dword[ebp + 8] ; push size as parameter
    int 0x80
    add esp, 4; restore stack pointer
    pop ebp
    ret

; void firstos_free(void* ptr)
firstos_free:
    push ebp
    mov ebp, esp
    mov eax, 5 ; free system call. allocat memory to process
    push dword[ebp + 8] ; ptr variable
    int 0x80
    add esp, 4; restore stack pointer
    pop ebp
    ret
