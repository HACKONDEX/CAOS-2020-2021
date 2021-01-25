.global solve
solve:
    push {lr}
    push {ip}
    mov ip, #-1
    for_start:
        add ip, ip, #1
        mov lr, #0
        mla lr, ip, r0, r1
        mla lr, ip, lr, r2
        mla lr, ip, lr, r3
        cmp lr, #0
        bne for_start
    mov r0, ip
    pop {ip}
    pop {pc}
