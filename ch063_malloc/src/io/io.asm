section .asm

global insb
global insw
global outb
global outw

; IN instruction
; https://c9x.me/x86/html/file_module_x86_id_139.html
; input from port
insb:
    push ebp
    mov ebp, esp

    xor eax, eax ; eax stores return value. XOR itself makes it 0(clear it)
    mov edx, [ebp+8] ; port is parameter. Then store that in edx
    in al, dx ; Read from port specified in DX(lower 16 bit in EDX) to AL(lower 8 bit[byte] in EAX).

    pop ebp
    ret ; return EAX

insw:
    push ebp
    mov ebp, esp

    xor eax, eax ; eax stores return value. XOR itself makes it 0(clear it)
    mov edx, [ebp+8] ; port is parameter. Then store that in edx
    in ax, dx ; Read from port specified in DX(lower 16 bit in EDX) to AX(lower 16 bit[word] in EAX).

    pop ebp
    ret ; return EAX

; https://www.felixcloutier.com/x86/out
outb:
    push ebp
    mov ebp, esp

    mov eax, [ebp+12] ; eax stores "val"
    mov edx, [ebp+8] ; edx stores "port"
    out dx, al ; output value in al (lower 8 bit[byte] of eax) to port specified in DX(lower 16 bit of EDX)

    pop ebp
    ret

outw:
    push ebp
    mov ebp, esp

    mov eax, [ebp+12] ; eax stores "val"
    mov edx, [ebp+8] ; edx stores "port"
    out dx, ax ; output value in ax (lower 16 bit[word] of eax) to port specified in DX(lower 16 bit of EDX)

    pop ebp
    ret