#define _GNU_SOURCE
#include <fcntl.h>
#include <malloc.h>
#include <semaphore.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <unistd.h>

typedef double (*function_t)(double);

double *pmap_process(function_t func, const double *in, size_t count) {
    double *out = mmap(NULL,
                       sizeof(double) * count,
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS,
                       -1, 0);
    const int process_count = get_nprocs();
    size_t each_proc_range = count / process_count;
    if (each_proc_range == 0) {
        each_proc_range = 1;
    }

    sem_t *sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(sem, 1, 0);

    pid_t *pids = calloc(count, sizeof(pid_t));
    size_t start = 0;
    size_t end = each_proc_range;
    for (int i = 0; i < process_count; ++i) {
        pids[i] = fork();
        if (pids[i] == 0) {
            size_t real_end = end <= count ? end : count;
            if (i == process_count - 1) {
                real_end = count;
            }
            for (size_t j = start; j < real_end; ++j) {
                out[j] = (*func)(in[j]);
            }
            sem_post(sem);
            return 0;
        }
        start = end;
        end += each_proc_range;
    }

    for (int i = 0; i < process_count; ++i) {
        sem_wait(sem);
    }

    for (int i = 0; i < process_count; ++i) {
        int status = 0;
        waitpid(pids[i], &status, 0);
    }

    sem_destroy(sem);
    return out;
}

void pmap_free(double *ptr, size_t count) {
    munmap(ptr, sizeof(double) * count);
}

