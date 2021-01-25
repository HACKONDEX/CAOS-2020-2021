    .section .text
    .global main
main:
    push {lr}
        while:
            ldr r0, =stdin
            ldr r0, [r0]
            bl fgetc
            cmp r0, #-1
            bne if_not_EOF
            b break_from_while
            if_not_EOF:
                mov r2, r0
                cmp r2, #47
                bgt if_greater_than_47
                b while
            if_greater_than_47:
                cmp r2, #58
                blt print_the_charachter
                b while
            print_the_charachter:
                ldr r1, =stdout
                ldr r1, [r1]
                bl fputc
                b while
        break_from_while:
            mov r0, #0
    pop {lr}
    bx lr
