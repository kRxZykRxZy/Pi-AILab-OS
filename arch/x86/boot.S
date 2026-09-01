.section .text
.global _start
.code32
_start:
    cli
    mov $stack_top, %esp
    call kernel_main
1:  hlt
    jmp 1b
.section .bss
.align 16
stack:
    .skip 16384
stack_top:
