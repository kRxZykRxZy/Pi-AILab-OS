.section .text
.global arm32_enter_user
.type arm32_enter_user,%function
arm32_enter_user:
    msr cpsr_c, #0x10
    mov sp, r1
    bx r0
1:  b 1b
