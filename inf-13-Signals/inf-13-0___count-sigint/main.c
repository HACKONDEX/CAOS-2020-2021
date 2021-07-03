#include <signal.h>
#include <stdio.h>
#include <unistd.h>

volatile int sigint_count = 0;
volatile int is_signal_sigterm = 0;

static void sigint_handler(int signum) {
    ++sigint_count;
}

static void sigterm_handler(int signum) {
    is_signal_sigterm = 1;
}

int main() {

    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    sigaction(SIGINT,
              &(struct sigaction){
                  .sa_handler = sigint_handler,
                  .sa_flags = SA_RESTART},
              NULL);

    sigaction(SIGTERM,
              &(struct sigaction){
                  .sa_handler = sigterm_handler,
                  .sa_flags = SA_RESTART},
              NULL);

    sigdelset(&mask, SIGINT);
    sigdelset(&mask, SIGTERM);

    printf("%i\n", getpid());
    fflush(stdout);

    while (is_signal_sigterm == 0) {
        sigsuspend(&mask);
    }

    printf("%i\n", sigint_count);

    return 0;
}

