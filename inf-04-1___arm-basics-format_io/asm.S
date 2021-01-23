    .section .text
    .global main
scanf_format:
    .ascii "%i%i\0"
printf_format:
    .ascii "%i\0"
main:
    push {lr}
        //scanf
        sub sp, #8
        ldr r0, =scanf_format
        mov r1, sp
        mov r2, sp
        add r2, #4
        bl scanf
        //multiplicate
        ldr r1, [sp]
        ldr r2, [sp, #4]
        add r1, r2, r1
        add sp, #8
        ldr r0, =printf_format
        bl printf
        mov r0, #0
    pop {lr}
    bx lr
