.section .text.boot
.global _start
.extern kernel_main64
_start:
    msr daifset, #0xf
    ldr x0, =stack_top
    mov sp, x0
    bl kernel_main64
1:  wfe
    b 1b

.section .bss
.align 12
stack:
    .space 16384
stack_top:
