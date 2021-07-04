#define _GNU_SOURCE
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

sig_atomic_t finish = 0;

static void sigterm_handler(int signum) {
    finish = 1;
}

int main() {
    sigaction(SIGTERM,
              &(struct sigaction){
                  .sa_handler = sigterm_handler,
                  .sa_flags = SA_RESTART},
              NULL);

    pid_t pids[3];

    const char symbols[3] = {'T', 'P', 'M'};
    sem_t *smoke_sem = mmap(NULL, sizeof(sem_t) * 3, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_t *wait_sem = mmap(NULL, sizeof(sem_t) * 3, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    for (int i = 0; i < 3; ++i) {
        sem_init(smoke_sem + i, 1, 0);
        sem_init(wait_sem + i, 1, 1);
    }

    for (int i = 0; i < 3; ++i) {
        pids[i] = fork();
        if (pids[i] == 0) {
            int index = i;

            while (finish != 1) {
                sem_wait(smoke_sem + index);
                if (finish == 1) {
                    break;
                }
                printf("%c", symbols[index]);
                fflush(stdout);
                sem_post(wait_sem + index);
            }
            return 0;
        }
    }

    char new_symbol = 0;
    while (scanf("%c", &new_symbol) > 0) {
        int proc_index = -1;
        switch (new_symbol) {
            case 't':
                proc_index = 0;
                break;
            case 'p':
                proc_index = 1;
                break;
            case 'm':
                proc_index = 2;
                break;
        }
        if(proc_index == -1) {
            break;
        }
        sem_wait(wait_sem + proc_index);
        sem_post(smoke_sem + proc_index);
    }

    for (int i = 0; i < 3; ++i) {
        sem_wait(wait_sem + i);
        kill(pids[i], SIGTERM);
        sem_post(smoke_sem + i);
    }

    for (int i = 0; i < 3; ++i) {
        int status = 0;
        waitpid(pids[i], &status, 0);
    }

    for (int i = 0; i < 3; ++i) {
        sem_destroy(smoke_sem + i);
        sem_destroy(wait_sem + i);
    }
    munmap(smoke_sem, sizeof(sem_t) * 3);
    munmap(wait_sem, sizeof(sem_t) * 3);
    return 0;
}

