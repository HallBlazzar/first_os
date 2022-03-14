section .asm

extern int21h_handler
extern no_interrupt_handler

global int21h
global idt_load
global no_interrupt
global enable_interrupts
global disable_interrupts

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
    cli ; clear(disable) interrupt. Prevent other interrupts occur during calling interrupt

    ; https://faydoc.tripod.com/cpu/pushad.htm
    ; Pushes the contents of the general-purpose registers onto the stack
    pushad
    call no_interrupt_handler
    ; https://www.felixcloutier.com/x86/popa:popad
    ; Pop double word(ad) from stack to General-Purpose Registers
    popad

    sti ; set(enable) interrupt

    iret