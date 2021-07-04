#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    const char *first_command = argv[1];
    const char *second_command = argv[2];

    int pipe_fd[2];
    pipe(pipe_fd);

    pid_t first_child = fork();
    if (first_child == 0) {
        dup2(pipe_fd[1], 1);
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        execlp(first_command, first_command, NULL);
    }

    close(pipe_fd[1]);

    pid_t second_child = fork();
    if (second_child == 0) {
        dup2(pipe_fd[0], 0);
        close(pipe_fd[0]);
        execlp(second_command, second_command, NULL);
    }

    close(pipe_fd[0]);

    int status = 0;
    waitpid(first_child, &status, 0);
    waitpid(second_child, &status, 0);

    return 0;
}

