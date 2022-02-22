ORG 0x7c00 ; the memory address the program expect to be loaded
BITS 16 ; 16 bit mode

; video teletype output - http://www.ctyme.com/intr/rb-0106.htm
; configure all register to the register required on the page
start:
    mov ah, 0eh
    mov al, 'A'
    int 0x10 ; trigger interrupt

    jmp $ ; "$" stands for the current instruction. So it will lead infinitely loop there in runtime

; for first 510 byte data excluding the part already have data ('start' section),
; fill rest of them with '0'
times 510-($ - $$) db 0

; add signature required by bios on 511 and 512 byte
dw 0xAA55 ; 511-> AA, 512 -> 55