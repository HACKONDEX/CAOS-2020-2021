.intel_syntax noprefix
    .text
    .globl calc
calc:
    // calc(double A, double B, double C, int D)
    // (A + B) / (C + D)

    endbr64
    addsd xmm0, xmm1 // xmm1 = A + B
    pxor xmm1, xmm1
    cvtsi2sd xmm1, edi // xmm0 = D
    addsd xmm1, xmm2 // xmm0 = D + C
    divsd xmm0, xmm1
    ret

    .globl vector_sum
vector_sum:
    // vector_sum(int N, const float *A, const float *B, float *R)
    // rdi - N; rsi - A; rdx - B; rcx - R;
    mov rax, 0
.for:
    cmp rdi, 0
    jle .return
    add rdi, -4
    movaps xmm0, XMMWORD PTR [rsi+rax] // xmm0 = A[i, i+4)
    addps xmm0, XMMWORD PTR [rdx+rax]
    movaps XMMWORD PTR [rcx+rax], xmm0 // R[i, i+4) = sum[i, i+4)
    add rax, 16
    jmp .for
.return:
    xor rax, rax
    ret
