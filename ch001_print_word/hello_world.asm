ORG 0x7c00 ; the memory address the program expect to be loaded
BITS 16 ; 16 bit mode

; video teletype output - http://www.ctyme.com/intr/rb-0106.htm
; configure all register to the register required on the page
start:
    mov si, message ; move the address of the label, "message" into the si register
    call print
    jmp $ ; "$" stands for the current instruction. So it will lead infinitely loop there in runtime

print:
.loop:
    lodsb ; load byte at the address SI register stores into AL register, the move address SI stores to next byte
    cmp al, 0 ; compare content in al and 0 -> check end of string reached
    je .done ; je jumps to a label if the previous comparison was equal.
    call print_char
    jmp .loop
.done: ; "." indicates sub-label
    ret

print_char:
    mov ah, 0eh
    int 0x10 ; trigger interrupt
    ret ; return

; put a series of byte there, and end with 0. 0 used to indicate end of string
message:
    db 'wwwwwwwww hh', 0

; for first 510 byte data excluding the part already have data ('start' section),
; fill rest of them with '0'
times 510-($ - $$) db 0

; add signature required by bios on 511 and 512 byte
dw 0xAA55 ; 511-> AA, 512 -> 55