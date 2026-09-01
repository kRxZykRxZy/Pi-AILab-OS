.syntax unified
.cpu cortex-a7
.text
.align 2
.global arm32_mmu_enable
.type arm32_mmu_enable, %function
arm32_mmu_enable:
    push {r4-r7, lr}
    ldr r4, =arm32_l1_table
    mov r5, #0
1:
    mov r7, r5, lsl #20
    mov r0, #0xC00
    orr r7, r7, r0
    orr r7, r7, #2
    str r7, [r4, r5, lsl #2]
    add r5, r5, #1
    cmp r5, #1024
    blt 1b
    mov r0, #1
    mcr p15, 0, r0, c3, c0, 0
    mov r0, r4
    mcr p15, 0, r0, c2, c0, 0
    mov r0, #0
    mcr p15, 0, r0, c8, c7, 0
    mrc p15, 0, r0, c1, c0, 0
    orr r0, r0, #(1 << 0)
    orr r0, r0, #(1 << 1)
    orr r0, r0, #(1 << 2)
    mcr p15, 0, r0, c1, c0, 0
    mov r0, #0
    mcr p15, 0, r0, c7, c5, 4
    pop {r4-r7, pc}
.section .bss
.align 14
.global arm32_l1_table
arm32_l1_table:
    .space 16384
