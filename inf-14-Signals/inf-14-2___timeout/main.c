#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

pid_t pid = 0;
int timeout_flag = 0;
int sigchild_flag = 0;

static void sigalrm_handler(int signum) {
    timeout_flag = 1;
}

static void sigchild_handler(int signum) {
    sigchild_flag = 1;
}

void init_handlers() {
    sigaction(SIGALRM,
              &(struct sigaction){
                  .sa_handler = sigalrm_handler,
                  .sa_flags = SA_RESTART},
              NULL);
    sigaction(SIGCHLD,
              &(struct sigaction){
                  .sa_handler = sigchild_handler,
                  .sa_flags = SA_RESTART},
              NULL);
}

int main(int argc, char **argv) {
    sigset_t full_mask;
    sigfillset(&full_mask);
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGALRM);
    sigdelset(&mask, SIGCHLD);

    init_handlers();

    char *arguments[128] = {NULL};
    for (int i = 2; i <= argc - 1; ++i) {
        arguments[i - 2] = argv[i];
    }
    arguments[argc - 2] = NULL;

    int time_in_seconds = strtoll(argv[1], NULL, 10);
    int status = 0;

    alarm(time_in_seconds);

    pid = fork();
    if (pid == 0) {
        execvp(argv[2], arguments);
    }

    sigprocmask(SIG_BLOCK, &mask, NULL);
    sigsuspend(&mask);
    int return_code = 0;

    if(timeout_flag == 1) {
        printf("timeout\n");
        kill(pid, SIGTERM);
        return_code = 2;
    }

    pid_t child_proccess = waitpid(pid, &status, 0);

    if(return_code == 2) {
        return return_code;
    }

    if(WIFSIGNALED(status)) {
        printf("signaled\n");
        return 1;
    }


    printf("ok\n");
    return 0;
}
