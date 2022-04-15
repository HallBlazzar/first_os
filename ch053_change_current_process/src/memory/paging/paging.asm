[BITS 32]

section .asm

global load_page_table_directory
global enable_paging

load_page_table_directory:
    push ebp
    mov ebp, esp
    mov eax, [ebp + 8]
    mov cr3, eax ; cr3 register should contain the page directory to load
    pop ebp
    ret

enable_paging:
    push ebp
    mov ebp, esp
    mov eax, cr0 ; keep cr0 in eax
    or eax, 0x80000000 ; enable paging
    mov cr0, eax ; push back cr0 to make it take effect
    pop ebp
    ret
