[BITS 32]

global _start
extern main

section .asm ;; all codes are under .asm section after compiling

_start:
    call main
    ret
