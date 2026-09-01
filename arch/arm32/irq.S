.section .text
.global irq_vector
.type irq_vector,%function
irq_vector:
    sub lr, lr, #4
    stmdb sp!, {r0-r12, lr}
    bl arm32_irq_dispatch
    ldmia sp!, {r0-r12, lr}
    subs pc, lr, #0
