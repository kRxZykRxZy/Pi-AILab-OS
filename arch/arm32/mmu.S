.syntax unified
.cpu arm1176jzf-s
.text
.align 2
.global arm32_mmu_enable
.type arm32_mmu_enable, %function

/*
 * ARMv6/v7 short-descriptor MMU bootstrap.
 * Identity maps the first 1 GiB with 1 MiB sections. This is deliberately
 * conservative for V0.1: devices remain directly addressable while the
 * kernel gains an MMU-backed address space.
 */
arm32_mmu_enable:
    push {r4-r7, lr}

    ldr r4, =arm32_l1_table
    mov r5, #0
    mov r6, #0
1:
    /* Section descriptor: base | AP=RW | domain=0 | section bit. */
    mov r7, r5, lsl #20
    orr r7, r7, #0xC02
    str r7, [r4, r5, lsl #2]
    add r5, r5, #1
    cmp r5, #1024
    blt 1b

    /* Domain 0 = manager. */
    mov r0, #1
    mcr p15, 0, r0, c3, c0, 0

    /* Translation table base, TTBR0. */
    mov r0, r4
    mcr p15, 0, r0, c2, c0, 0

    /* Invalidate TLB. */
    mov r0, #0
    mcr p15, 0, r0, c8, c7, 0

    /* Enable MMU + alignment checking + data/unified cache. */
    mrc p15, 0, r0, c1, c0, 0
    orr r0, r0, #(1 << 0)
    orr r0, r0, #(1 << 1)
    orr r0, r0, #(1 << 2)
    mcr p15, 0, r0, c1, c0, 0
    isb

    pop {r4-r7, pc}

.section .bss
.align 14
.global arm32_l1_table
arm32_l1_table:
    .space 16384
