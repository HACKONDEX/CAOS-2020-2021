.global f
f:
    mla r2, r1, r3, r2
    mul r1, r0, r3
    mla r0, r1, r3, r2
    bx lr
