    .section .text
    .global main
main:
    push {r4, r5, r6, r7, r9}
    push {lr}
        mov r6, #0
        mov r5, #4
        mov r0, r5
        bl malloc
        mov r9, r0
        while_do:
            ldr r0, =stdin
            ldr r0, [r0]
            bl fgetc
            mov r7, r0
            cmp r7, #-1
            beq .print_the_symbols
            cmp r6, r5
            blt .add_symbol
            b .do_realloc

        .add_symbol:
            str r7, [r9, r6]
            add r6, r6, #1
            b while_do

        .do_realloc:
            mov r2, #2
            mul r5, r2, r5
            mov r1, r5
            mov r0, r9
            bl realloc
            mov r9, r0
            b .add_symbol


        .print_the_symbols:
            cmp r6, #0
            beq .go_and_do_free
            add r6, r6, #-1
            ldr r7, [r9, r6]
            mov r0, r7
            ldr r1, =stdout
            ldr r1, [r1]
            bl fputc
            b .print_the_symbols

        .go_and_do_free:
            mov r0, r9
            bl free
            mov r0, #0
            pop {lr}
            pop {r4, r5, r6, r7, r9}
            bx lr
