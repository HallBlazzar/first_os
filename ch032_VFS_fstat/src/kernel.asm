[BITS 32] ; make codes under the label be treated as 32 bit code

global _start_kernel
global trigger_divide_by_zero
extern kernel_main

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start_kernel:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; enable A20 line
    ; https://stackoverflow.com/questions/3215878/what-are-in-out-instructions-in-x86-used-for
    in al, 0x92 ; read from port 0x92 (on bus) to "al" register
    or al, 2 ; logical OR
    out 0x92, al ; write value in "al" register to port 0x92 (on bus)

    ; Remap the master PIC(port 0x20)
    mov al, 00010001b ; flag
    out 0x20, al ; Send flag to master PIC

    mov al, 0x20 ; Interrupt 0x20 is where master ISR should start
    out 0x21, al

    mov al, 00000001b
    out 0x21, al

    ; Run kernel
    call kernel_main

    jmp $

; fill 0 until reaching 512 byte
; when compile pure assembly with C code, compiler assumes each object file
; should be aligned as certain format. Otherwise, codes might be placed in
; wrong place, and processor might points to wrong instructions in runtime.
; The 512 byte filling(size of 16 bytes) could be treated as alignment for
; kernel.asm and kernel.asm.o
; Then when GCC compile it with other C code, it will be treated as well
; aligned code and could be loaded into memory correctly.
times 512-($ - $$) db 0
