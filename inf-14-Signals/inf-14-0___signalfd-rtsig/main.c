#include <ftw.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int max_line_length = 4096;
volatile int finish_reading = 0;
volatile int file_index = 1;

static void general_handler(int signum) {
    file_index = signum - SIGRTMIN;
}

static void sigrtmin_handler(int signum) {
    _exit(0);
}

int main(int argc, char *argv[]) {
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    for (int signal = SIGRTMIN; signal <= SIGRTMAX; ++signal) {
        sigdelset(&mask, signal);
    }

    sigaction(SIGRTMIN,
              &(struct sigaction){
                  .sa_handler = sigrtmin_handler,
                  .sa_flags = SA_RESTART},
              NULL);

    struct sigaction act;
    act.sa_handler = general_handler;
    act.sa_flags = SA_RESTART;

    for (int signal = SIGRTMIN + 1; signal <= SIGRTMAX; ++signal) {
        sigaction(signal, &act, NULL);
    }

    FILE *file_list[argc + 1];
    for (int i = 1; i <= argc; ++i) {
        file_list[i] = fopen(argv[i], "r");
    }

    char line[max_line_length];
    while (finish_reading != 1) {
        sigsuspend(&mask);
        if (file_index >= 1) {
            fgets(line, max_line_length, file_list[file_index]);
            printf("%s", line);
            fflush(stdout);
            continue;
        }
        break;
    }

    return 0;
}

