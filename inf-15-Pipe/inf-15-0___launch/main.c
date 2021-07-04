#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>

const int max_size = 1024;
int child_finish = 0;

static void sigchild_handler(int signum) {
    child_finish = 1;
}

int get_read_bytes_count(int fd, char *buffer) {
    int output_bytes_count = 0;
    int tmp_read_bytes = 0;

    do {
        tmp_read_bytes = read(fd, buffer, max_size);
        if (tmp_read_bytes <= 0) {
            break;
        }
        output_bytes_count += tmp_read_bytes;
    } while (tmp_read_bytes > 0);

    return output_bytes_count;
}

int main(int argc, char *argv[]) {

    sigaction(SIGCHLD,
              &(struct sigaction){
                  .sa_handler = sigchild_handler,
                  .sa_flags = SA_RESTART},
              NULL);

    char *command = argv[1];
    char *input = argv[2];

    int pipe_fd[2];
    pipe(pipe_fd);

    pid_t pid = fork();
    if (pid == 0) {
        int input_fd = open(input, O_RDONLY);
        dup2(input_fd, 0);
        dup2(pipe_fd[1], 1);
        execlp(command, command, NULL);
    }
    close(pipe_fd[1]);

    char tmp_buffer[max_size];
    int output_bytes_count = 0;

    while (child_finish == 0) {
        output_bytes_count += get_read_bytes_count(pipe_fd[0], tmp_buffer);
    }

    int status = 0;
    waitpid(pid, &status, 0);

    output_bytes_count += get_read_bytes_count(pipe_fd[0], tmp_buffer);

    printf("%d\n", output_bytes_count);
    return 0;
}

