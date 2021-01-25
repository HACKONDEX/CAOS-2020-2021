#include <sys/syscall.h>

long syscall(long number, ...);

void _start() {
    int is_it_end_of_file = 0;
    int return_code;
    char symbol;
    while (is_it_end_of_file != 1) {
        return_code = syscall(SYS_read, 0, &symbol, 1);
        if (return_code == 0) {
            is_it_end_of_file = 1;
            continue;
        }
        syscall(SYS_write, 1, &symbol, 1);
    }
    syscall(SYS_exit_group, 0);
}

