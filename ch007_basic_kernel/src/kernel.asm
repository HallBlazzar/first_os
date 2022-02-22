[BITS 32] ; make codes under the label be treated as 32 bit code
global _start_kernel

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start_kernel:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ; just test could stack pointers points to higher memory space without problem
    mov ebp, 0x00200000
    mov esp, ebp

    ; enable A20 line
    ; https://stackoverflow.com/questions/3215878/what-are-in-out-instructions-in-x86-used-for
    in al, 0x92 ; read from port 0x92 (on bus) to "al" register
    or al, 2 ; logical OR
    out 0x92, al ; write value in "al" register to port 0x92 (on bus)
    jmp $