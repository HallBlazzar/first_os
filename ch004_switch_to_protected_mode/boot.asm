ORG 0x7c00
BITS 16 ; 16 bit mode

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; first 3 bytes are meaningful. They will indicate where to start execute
_bios_parameter:
    jmp short _set_code_segment
    nop

    ; fill other parameters with value 0
    times 33 db 0

_set_code_segment:
    jmp 0:start ; set code segment with 0, and set instruction pointer(ip) with address of "start"

; video teletype output - http://www.ctyme.com/intr/rb-0106.htm
; configure all register to the register required on the page
start:

    ; Clear interrupt. It will disable interrupt. It make sure segment initialization progress won't be interrupted and cause unexpected result
    cli

    ; cannot directly assign value to ds and es, so we need to copy from ax
    mov ax, 0x00
    mov ds, ax ; data segment
    mov es, ax ; extra segment

    ; initialize stack segment. Note stack memory address decrease when pushing value to it(SP).
    mov ss, ax ; set stack segment 0
    mov sp, 0x7c00 ; set stack pointer to our target offset

    ; Enable interrupt
    sti

; enter protected mode
; https://wiki.osdev.org/Protected_Mode
.load_protected_label:
    cli
    lgdt[gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:load32

; GDT
gdt_start:
gdt_null:
    dd 0x0 ; (declare double word)
    dd 0x0

; offset 0x08
; https://wiki.osdev.org/Global_Descriptor_Table
; see Table and Segment Descriptor sections for content of each entry
gdt_code: ; CS should points to there
    dw 0xffff ;  0-15 bits for segment limit(0-15) (declare word)
    dw 0 ; 0-15 bits of base(16-31) (declare word)
    db 0 ; base 16-23 bits (32-39) (declare byte)

    db 0x9a ; access byte(40-47) (declare byte)
    db 11011111b ; high 4 bit for flags(52-55) and low 4bit for limit(48-51). "1111" stands for "F"
    db 0 ; base 24-31 bits(56-63)

; offset 0x10
gdt_data: ; should link to DS, SS, ES, FS, GS
    dw 0xffff ;  0-15 bits for segment limit(0-15) (declare word)
    dw 0 ; 0-15 bits of base(16-31) (declare word)
    db 0 ; base 16-23 bits (32-39) (declare byte)

    db 0x92 ; access byte(40-47) (declare byte)
    db 11011111b ; high 4 bit for flags(52-55) and low 4bit for limit(48-51). "1111" stands for "F"
    db 0 ; base 24-31 bits(56-63)

gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; GDT descriptor size
    dd gdt_start

[BITS 32] ; make codes under the label be treated as 32 bit code
load32:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ; just test could stack pointers points to higher memory space without problem
    mov ebp, 0x00200000
    mov esp, ebp
    jmp $

; for first 510 byte data excluding the part already have data ('start' section),
; fill rest of them with '0'
times 510-($ - $$) db 0

; add signature required by bios on 511 and 512 byte
dw 0xAA55 ; 511-> AA, 512 -> 55

