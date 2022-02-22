ORG 0
BITS 16 ; 16 bit mode

; first 3 bytes are meaningful. They will indicate where to start execute
_bios_parameter:
    jmp short _set_code_segment
    nop

    ; fill other parameters with value 0
    times 33 db 0

_set_code_segment:
    jmp 0x7c0:start ; set code segment with 0x7c0, and set instruction pointer(ip) with address of "start"

; video teletype output - http://www.ctyme.com/intr/rb-0106.htm
; configure all register to the register required on the page
start:

    ; Clear interrupt. It will disable interrupt. It make sure segment initialization progress won't be interrupted and cause unexpected result
    cli

    ; cannot directly assign value to ds and es, so we need to copy from ax
    mov ax, 0x7c0
    mov ds, ax ; data segment
    mov es, ax ; extra segment

    ; initialize stack segment. Note stack memory address decrease when pushing value to it(SP).
    mov ax, 0x00
    mov ss, ax ; set stack segment 0
    mov sp, 0x7c00 ; set stack pointer to our target offset

    ; Enable interrupt
    sti

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