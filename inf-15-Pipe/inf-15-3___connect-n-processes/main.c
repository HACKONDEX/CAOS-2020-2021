#define _GNU_SOURCE

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>


const int max_size = 1024;

int main(int argc, char *argv[]) {
    if (argc == 1) {
        return 0;
    } else if (argc == 2) {
        execlp(argv[1], argv[1], NULL);
        _exit(1);
    }

    int commands_count = argc - 1;
    int pipe_fd[max_size][2];
    pid_t child_pid[max_size];

    for (int i = 1; i < commands_count; ++i) {
        pipe2(pipe_fd[i], O_CLOEXEC);
    }

    child_pid[1] = fork();
    if (child_pid[1] == 0) {
        dup2(pipe_fd[1][1], 1);
        close(pipe_fd[1][0]);
        execlp(argv[1], argv[1], NULL);
    }

    for (int i = 2; i < commands_count; ++i) {
        child_pid[i] = fork();
        if (child_pid[i] == 0) {
            dup2(pipe_fd[i][1], 1);
            close(pipe_fd[i][0]);
            dup2(pipe_fd[i - 1][0], 0);
            close(pipe_fd[i - 1][1]);
            execlp(argv[i], argv[i], NULL);
        }
    }

    child_pid[commands_count] = fork();
    if (child_pid[commands_count] == 0) {
        dup2(pipe_fd[commands_count - 1][0], 0);
        close(pipe_fd[commands_count - 1][1]);
        execlp(argv[commands_count], argv[commands_count], NULL);
    }

    for (int i = 1; i <= commands_count; ++i) {
        int status = 0;
        waitpid(child_pid[i], &status, 0);
        if (i != commands_count)
            close(pipe_fd[i][0]);
        close(pipe_fd[i][1]);
    }
    return 0;
}

