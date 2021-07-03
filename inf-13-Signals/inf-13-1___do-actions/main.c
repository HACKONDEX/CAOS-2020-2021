#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

volatile int signal_number = 0;

static void sigusr1_handler(int signum) {
    signal_number = 1;
}

static void sigusr2_handler(int signum) {
    signal_number = 2;
}

static void sigint_sigterm_handler(int signum) {
    signal_number = 3;
}

void sigaction_change() {
    sigaction(SIGUSR1,
              &(struct sigaction){
                  .sa_handler = sigusr1_handler,
                  .sa_flags = SA_RESTART},
              NULL);

    sigaction(SIGUSR2,
              &(struct sigaction){
                  .sa_handler = sigusr2_handler,
                  .sa_flags = SA_RESTART},
              NULL);

    sigaction(SIGINT,
              &(struct sigaction){
                  .sa_handler = sigint_sigterm_handler,
                  .sa_flags = SA_RESTART},
              NULL);

    sigaction(SIGTERM,
              &(struct sigaction){
                  .sa_handler = sigint_sigterm_handler,
                  .sa_flags = SA_RESTART},
              NULL);
}

int main() {

    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    sigaction_change();

    sigdelset(&mask, SIGINT);
    sigdelset(&mask, SIGTERM);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGUSR2);

    printf("%i\n", getpid());
    fflush(stdout);

    int32_t number = 0;
    scanf("%i", &number);

    int sigterm_or_sigint = 0;
    while (sigterm_or_sigint == 0) {
        sigsuspend(&mask);
        switch (signal_number) {
            case 1:
                ++number;
                printf("%i\n", number);
                fflush(stdout);
                break;
            case 2:
                number *= -1;
                printf("%i\n", number);
                fflush(stdout);
                break;
            case 3:
                sigterm_or_sigint = 1;
                break;
        }
        signal_number = 0;
    }

    return 0;
}

