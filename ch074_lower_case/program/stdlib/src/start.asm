[BITS 32]

global _start
extern c_start
extern firstos_exit

section .asm ;; all codes are under .asm section after compiling

_start:
    call c_start
    call firstos_exit
    ret
