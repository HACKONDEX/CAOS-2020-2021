    .intel_syntax noprefix
    .global _start
    .text
_start:
    sub rsp, 8
.while:
    mov rax, 0
    mov rdi, 0
    mov rsi, rsp
    mov rdx, 1
    syscall
    cmp rax, 0
    je .exit_group
    mov rax, 1
    mov rdi, 1
    mov rsi, rsp
    mov rdx, 1
    syscall
    jmp .while
.exit_group:
    mov rax, 231
    mov rdi, 0
    syscall
