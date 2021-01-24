  .intel_syntax noprefix
    .text
.format:
    .string "%d"
.format_ptr:
    .quad .format
    .globl summ
summ:
    // edi - current index
    // edx - A
    // ecx - B
    // edi - R

    mov rdi, N
    mov rdx, A
    mov rcx, B
    mov rsi, R
.for:
    cmp rdi, 0
    jle .return
    dec rdi

    mov rax, [rdx]
    add rax, [rcx]
    mov [rsi], rax

    add rdx, 4
    add rcx, 4
    add rsi, 4

    jmp .for
.return:
    ret

    .globl everyday795
everyday795:
    ////// x * input + y
    // X = rdi = rbp
    // Y = rsi = rbx
    // ret - 8
    push r12 // 16
    push r13 // 24

    mov r12, rdi
    mov r13, rsi

    // сдвинуть стек пойнтер(локальная переменная)
    sub rsp, 8 // 32

    mov rdi, .format_ptr
    mov rsi, rsp
    call scanf

    mov rax, r12
    mov rcx, [rsp]
    imul rax, rcx
    add rax, r13

    mov rsi, rax
    mov rdi, .format_ptr
    call printf

    add rsp, 8
    pop r13
    pop r12
    ret

