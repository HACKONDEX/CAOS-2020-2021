    .section .data
    .global R
R:
    .space 4
    .global calculate
calculate:
        ldr r0, variables
        ldr r0, [r0]
        ldr r1, variables+4
        ldr r1, [r1]
        ldr r2, variables+8
        ldr r2, [r2]
        ldr r3, variables+12
        ldr r3, [r3]
        mul r0, r1, r0
        mul r2, r3, r2
        add r0, r2, r0
        ldr r1, variables+16
        str r0, [r1]
    bx lr
variables:
    .word A
    .word B
    .word C
    .word D
    .word R
