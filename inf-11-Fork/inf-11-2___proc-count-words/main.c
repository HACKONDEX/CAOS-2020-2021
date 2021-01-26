#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

const int max_word_size = 4096;

int main() {
    int process_number = 0;
    int read_word_count = 0;
    pid_t current_pid = 0;
    char buffer[max_word_size];
    int can_read = 1;
    int success = 0;
    while (can_read == 1) {
        current_pid = fork();
        if (current_pid == 0) {
            ++process_number;
            success = scanf("%s", buffer);
            if (success == -1) {
                can_read = 0;
            } else {
                ++read_word_count;
            }
            continue;
        }
        can_read = 0;
    }

    int status = 0;
    waitpid(current_pid, &status, 0);

    if (success == -1) {
        return read_word_count;
    }

    read_word_count = WEXITSTATUS(status);
    if (process_number != 0) {
        return read_word_count;
    }

    printf("%i\n", read_word_count);
    return 0;
}
