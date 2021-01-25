    .intel_syntax noprefix
    .text
    .globl dot_product
dot_product:
    endbr64
    /*
        rdi = N
        rsi = A
        rdx = B
    */
    mov rax, 0
    cvtsi2sd xmm0, rax
    mov rcx, 0
.for:
    cmp rdi, 4
    jle .three_or_less_case
    add rdi, -4
    movups xmm1, XMMWORD PTR [rsi+rcx]
    movups xmm2, XMMWORD PTR [rdx+rcx]
    dpps xmm1, xmm2, 0xF1
    addss xmm0, xmm1
    add rcx, 16
    jmp .for
.three_or_less_case:
    cmp rdi, 0
    je .return
    add rdi, -1
    movss xmm1, DWORD PTR [rsi+rcx]
    movss xmm2, DWORD PTR [rdx+rcx]
    mulss xmm1, xmm2
    addss xmm0, xmm1
    add rcx, 4
    jmp .three_or_less_case
.return:
    ret
