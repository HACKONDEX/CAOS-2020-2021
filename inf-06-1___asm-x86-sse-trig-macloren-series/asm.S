.intel_syntax noprefix
    .text
    .globl my_sin
my_sin:
    endbr64
    /*
        xmm0 = answer
        xmm1 = previous_answer
        xmm2 = -x^2
        rsi = i
        xmm4 = current_summand
        current_summand *= -(x^2)/((i+1)(i+2))
    */

    movapd xmm4, xmm0
    //movsd xmm2, QWORD PTR .constants[rip]
    mov rax, -1
    cvtsi2sd xmm2, rax
    mulsd xmm2, xmm4
    mulsd xmm2, xmm4
    mov rsi, 1
    mov rdi, 0
    cvtsi2sd xmm1, rdi
.while:
    comisd xmm0, xmm1
    je .ret
    movapd xmm1, xmm0
    mulsd xmm4, xmm2
    add rsi, 1
    cvtsi2sd xmm3, rsi
    divsd xmm4, xmm3
    add rsi, 1
    cvtsi2sd xmm3, rsi
    divsd xmm4, xmm3
    addsd xmm0, xmm4
    jmp .while
.ret:
    ret
