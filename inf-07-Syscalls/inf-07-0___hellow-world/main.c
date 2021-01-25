#include <sys/syscall.h>

long syscall(long number, ...);

void _start() {
    syscall(SYS_write, 1, "Hello, World!", 13);
    syscall(SYS_exit_group, 0);
}
