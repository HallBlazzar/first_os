section .asm

extern int21h_handler
extern no_interrupt_handler
extern isr80h_handler

global int21h
global idt_load
global no_interrupt
global enable_interrupts
global disable_interrupts
global isr80h_wrapper

enable_interrupts:
    sti
    ret

disable_interrupts:
    cli
    ret

idt_load: ; load inturrpt descriptor table
    push ebp
    mov ebp, esp

    mov ebx, [ebp + 8] ; ebp + 8 points to address of fist argument of the funciton
    lidt [ebx] ; load idt

    pop ebp
    ret

; handler caller for int 21(ISA IRQ 1)
; we mapped IRQ 0 to int 20 in kernel.asm
; this is the wrapper for int21h_handler
int21h:
    cli ; clear(disable) interrupt. Prevent other interrupts occur during calling interrupt

    ; https://faydoc.tripod.com/cpu/pushad.htm
    ; Pushes the contents of the general-purpose registers onto the stack
    pushad
    call int21h_handler
    ; https://www.felixcloutier.com/x86/popa:popad
    ; Pop double word(ad) from stack to General-Purpose Registers
    popad

    sti ; set(enable) interrupt

    iret

; for interrupts with no handlers defined, they will fallback to use this handler instead.
; Otherwise, if processor receive an interrupt without handler, it will cause interrupt fault
; this is the wrapper for no_interrupt_handler
no_interrupt:
    ; https://faydoc.tripod.com/cpu/pushad.htm
    ; Pushes the contents of the general-purpose registers onto the stack
    pushad
    call no_interrupt_handler
    ; https://www.felixcloutier.com/x86/popa:popad
    ; Pop double word(ad) from stack to General-Purpose Registers
    popad

    sti ; set(enable) interrupt

    iret

; interrupt 0x80 handler
; isr stands for interrupt service routine
isr80h_wrapper:
    ; construct interrupt frame
    ; an interrupt frame consists of:
    ; uint32_t ip
    ; uint32_t cs
    ; uint32_t flags
    ; uint32_t sp;
    ; uint32_t ss;
    ; and general purpose registers (eax, edx, ... etc)
    pushad ; push general purpose registers to stack

    ; push stack pointer
    push esp ; --> move stack point to place contains all information in interrupt frame

    push eax ; eax contains system call code
    call isr80h_handler

    ; in C, return value will be stored in eax register if a value is a integer. So we move that value to a temporary variable to store it
    ; note that if the return value larger than an integer, then eax might not be used
    ; also, reason temporarily store eax in another place is to prevent eax from being used before return
    mov dword[temp_result], eax

    ; move stack pointer back to the place before preparing interrupt frame,
    ; so processor can continue execute origin task before interrupt
    ; the reason move 8 byte is esp and eax(line 78 and 80) are 4 bytes, so move 8 bytes offset is required
    add esp, 8

    ; restore general purpose registers for caller task/process
    ; reflects to pushad in line 75
    popad

    mov eax, [temp_result]

    iretd ; return from interrupt

section .data
; for isr80h_wrapper to store return result (eax) from isr80h handler
temp_result: dd 0