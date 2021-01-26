#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    const int max_elements_count = atoi(argv[1]);
    int elements_count = max_elements_count - 1;
    int current_number = max_elements_count;
    pid_t current_pid = 0;
    int is_parent_process = 0;
    while (elements_count > 0 && is_parent_process == 0) {
        current_pid = fork();
        if (current_pid == 0) {
            --elements_count;
            --current_number;
            continue;
        }
        is_parent_process = 1;
    }
    int status;
    waitpid(current_pid, &status, 0);
    printf("%i", current_number);
    char next_symbol = 0;
    next_symbol = (elements_count == max_elements_count - 1) ? '\n' : ' ';
    printf("%c", next_symbol);
    return 0;
}

