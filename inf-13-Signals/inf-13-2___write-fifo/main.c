#include <fcntl.h>
#include <ftw.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

volatile int finish_writng = 0;

void static sigpipe_handler() {
    finish_writng = 1;
}

int main(int argc, char **argv) {

    sigaction(SIGPIPE,
              &(struct sigaction){
                  .sa_handler = sigpipe_handler,
                  .sa_flags = SA_RESTART},
              NULL);

    const char *pipe_name = argv[1];
    int32_t max_number = strtol(argv[2], NULL, 10);

    mkfifo(pipe_name, 0644);

    pid_t other_process_pid = 0;
    scanf("%i", &other_process_pid);
    kill(other_process_pid, SIGHUP);

    int pipe_fd = open(pipe_name, O_WRONLY);

    int32_t current_number = 0;
    int32_t written_numbers_count = 0;
    int return_code = 0;
    char count[100];
    while (finish_writng != 1 && current_number <= max_number) {
        if (current_number == 0) {
            sprintf(count, "%i", current_number);
            return_code = write(pipe_fd, count, strlen(count));

            ++current_number;
        } else {
            sprintf(count, " %i", current_number);
            write(pipe_fd, count, strlen(count));
            ++current_number;
        }
        if (return_code <= 0) {
            break;
        } else {
            ++written_numbers_count;
        }
    }
    close(pipe_fd);
    printf("%i\n", written_numbers_count);
    return 0;
}

