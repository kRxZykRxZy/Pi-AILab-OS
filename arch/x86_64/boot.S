.section .text
.global _start
.code64
_start:
    cli
    lea stack_top(%rip), %rsp
    call kernel_main
1:  hlt
    jmp 1b
.section .bss
.align 16
stack:
    .skip 16384
stack_top:
