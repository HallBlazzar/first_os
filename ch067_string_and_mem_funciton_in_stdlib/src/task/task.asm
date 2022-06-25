[BITS 32]

section .asm

global return_task
global restore_general_purpose_registers
global change_to_user_data_register

; void return_task(struct registers* registers);
; load task related segments/stack/flag/ip
return_task:
    mov ebp, esp
    ; orders:
    ; 1. push data segment (ss will be fine)
    ; 2. push stack address
    ; 3. push flags
    ; 4. push code segment
    ; 5. push ip

    ; access structure passed to this function
    mov ebx, [ebp + 4]
    ; push data/stack selector
    push dword [ebx + 44] ; ss
    ; push stack pointer
    push dword [ebx + 40] ; esp

    ; push flags
    pushf ; flag
    pop eax
    or eax, 0x200 ; re-enable interrupt after iretd
    push eax

    ; push code segment
    push dword [ebx + 32] ; cs

    ; push ip to execute(virtual address)
    push dword [ebx + 28] ; ip

    ; setup segment registers
    mov ax, [ebx + 44]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push dword [ebp + 4]
    call restore_general_purpose_registers
    add esp, 4

    ; leave kernel land and enter user land
    iretd

; void restore_general_purpose_registers(struct registers* registers);
restore_general_purpose_registers:
    push ebp
    mov ebp, esp
    mov ebx, [ebp + 8]
    mov edi, [ebx]
    mov esi, [ebx + 4]
    mov ebp, [ebx + 8]
    mov edx, [ebx + 16]
    mov ecx, [ebx + 20]
    mov eax, [ebx + 24]
    mov ebx, [ebx + 12]
    pop ebp
    ret

; void change_to_user_data_register()
; change segment registers to user data registers
change_to_user_data_register:
    mov ax, 0x23 ; change segment selector to 0x23 --> offset of user data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret