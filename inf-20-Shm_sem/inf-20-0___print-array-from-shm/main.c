#define _GNU_SOURCE
#include <fcntl.h>
#include <inttypes.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

int main() {
    char semaphor_name[256] = {0};
    char shared_memory_name[256] = {0};
    uint32_t numbers_count = 0;
    scanf("%s%s%" PRIu32, semaphor_name, shared_memory_name, &numbers_count);

    sem_t *sem = sem_open(semaphor_name, 0);
    sem_wait(sem);

    int fd = shm_open(shared_memory_name, O_RDWR, 0644);

    int *array = mmap(NULL, numbers_count * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    for (int i = 0; i < numbers_count; ++i) {
        printf("%d\n", array[i]);
    }

    munmap(array, numbers_count * sizeof(int));
    close(fd);

    sem_post(sem);
    sem_close(sem);
    return 0;
}

