[BITS 32]

section .asm

global print:function
global firstos_get_key:function
global firstos_putchar:function
global firstos_malloc:function
global firstos_free:function
global firstos_start_load_process:function

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
firstos_get_key:
    push ebp
    mov ebp, esp
    mov eax, 2; get key system call. get keyboard key
    int 0x80
    pop ebp
    ret

; void firstos_putchar(char c)
firstos_putchar:
    push ebp
    mov ebp, esp
    mov eax, 3; putchar system call
    push dword [ebp + 8] ; c varible
    int 0x80
    add esp, 4
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

; void firstos_start_load_process(const char* filename)
firstos_start_load_process:
    push ebp
    mov ebp, esp
    mov eax, 6 ;  start load process system call.
    push dword[ebp + 8] ; "filename" variable
    int 0x80
    add esp, 4
    pop ebp
    ret
