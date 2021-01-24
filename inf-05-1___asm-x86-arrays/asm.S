    .intel_syntax noprefix
    .text
    .globl summ
summ:
    // int N - rdi
    // int* A - rsi
    // int* B - rdx
    // int* R - rcx
.for:
    cmp rdi, 0
    jle .return
    dec rdi

    mov rax, [rsi]
    add rax, [rdx]
    mov [rcx], rax

    add rsi, 4
    add rdx, 4
    add rcx, 4

    jmp .for
.return:
    ret
