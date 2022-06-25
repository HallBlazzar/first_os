[BITS 32]

global _start
extern c_start

section .asm ;; all codes are under .asm section after compiling

_start:
    call c_start
    ret
