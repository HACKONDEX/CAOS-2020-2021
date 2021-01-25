#include <sys/syscall.h>
    .intel_syntax noprefix
    .global _start
    .text
_start:
    sub rsp, 8
.allocate_memory_and_save_input:
    /* r12 - current pointer
       r13 - pointer to the end of buffer
       rbx - buffer size
       rbp - symbols count in buffer */
    mov rbx, 0
    mov rbp, 0
    mov rax, SYS_brk
    mov rdi, 0
    syscall // r12 = brk(0)
    mov r12, rax
    add rbx, 32
    mov rax, SYS_brk
    mov rdi, r12
    add rdi, rbx
    syscall // brk(r12 + 32)
    mov r13, rax
.while_can_read:
    mov rax, rbp
    add rax, 32
    cmp rbx, rax // check (buffer_size <= size + 32)
    jle .get_more_memory
    //read part
    mov rax, SYS_read
    mov rdi, 0
    mov rsi, r12
    mov rdx, 32
    syscall // read(1, r12, 32)
    cmp rax, 0 // if have read 0 symbols finish rreading
    jle .finish_reading_input
    add r12, rax // r12 += symbols_count_read
    add rbp, rax // size += symbols_count_read
    jmp .while_can_read

.get_more_memory:
    mov rax, SYS_brk
    mov rdi, r13
    add rdi, rbx
    syscall // brk(r13 + buffer_size)
    mov r13, rax
    add rbx, rbx // buffer_size *= 2
    jmp .while_can_read

.finish_reading_input:
    cmp rbp, 0 // if buffer is empty, finish programm
    jle .exit_group

.print_in_reverse_order:
    // last symbol check for '\n'
    sub r12, 1
    mov al, BYTE PTR [r12]
    add r12, 1
    cmp al, 10
    je .high_loop_when_string_starts
    mov BYTE PTR [r12], 10
    add r12, 1
    add rbp, 1

.high_loop_when_string_starts:
    mov r14, 0
    /*
        r14 - count of symbols before first '\n'
        rbp - symbols count left to print
    */

.while_not_new_line:
    sub r12, 1
    cmp rbp, 0 // if there are no symbols left, print and finish
    jle .print_current_string
    sub rbp, 1
    mov al, BYTE PTR [r12]
    cmp al, 10 // if symbol == '\n'
    je .print_current_string
    add r14, 1
    jmp .while_not_new_line

.print_current_string:
    // print symbols after '\n'
    add r12, 1
    mov rax, SYS_write
    mov rdi, 1
    mov rsi, r12
    mov rdx, r14
    syscall //write(1, r12, r14)
    sub r12, 1
    mov r14, 1
    cmp rbp, 0
    jle .exit_group
    jmp .while_not_new_line

.exit_group:
    mov rax, SYS_exit_group
    mov rdi, 0
    syscall
