    .global	summ
summ:
    mov	r3, #0
    cmp	r3, r1
    blt	do_for
    bx lr
do_for:
    push {ip}
    for_body:
        ldr ip, [r2, r3, lsl #2]
        add r0, ip, r0
        add r3, r3, #1
        cmp r3, r1
        blt for_body
    pop {ip}
    bx lr
