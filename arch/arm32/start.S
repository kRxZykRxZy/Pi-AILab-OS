.section .text.boot
.global _start
.extern kernel_main32
.extern arm32_mmu_enable
_start:
    cpsid if
    ldr sp, =stack_top

    /* Preserve firmware boot arguments while bringing up the MMU. */
    mov r4, r0
    mov r5, r1
    mov r6, r2
    bl arm32_mmu_enable
    mov r0, r4
    mov r1, r5
    mov r2, r6
    bl kernel_main32
1:  wfe
    b 1b

.section .bss
.align 12
stack:
    .space 16384
stack_top:
