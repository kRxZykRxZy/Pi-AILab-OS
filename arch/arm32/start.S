.section .text.boot
.global _start
.extern kernel_main32
_start:
    cpsid if
    ldr sp, =stack_top
    mov r0, r0
    mov r1, r1
    mov r2, r2
    bl kernel_main32
1:  wfe
    b 1b

.section .bss
.align 12
stack:
    .space 16384
stack_top:
