.section .text
.global sched_switch
.type sched_switch,%function
sched_switch:
    @ r0 = old context, r1 = new context
    stmia r0, {r4-r12, sp, lr}
    str r13, [r0, #52]
    ldmia r1, {r4-r12, sp, lr}
    ldr r13, [r1, #52]
    ldr r2, [r1, #56]
    bx r2
