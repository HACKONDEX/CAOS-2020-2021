#include <dlfcn.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct {
    sem_t request_ready; // 0
    sem_t response_ready;// 0
    char func_name[20];
    double value;
    double result;
} shared_data_t;

// returns 0 if no request, 1 otherwise
int process_request(shared_data_t *data, void *lib_handle) {
    sem_wait(&(data->request_ready));
    if (strlen(data->func_name) == 0) {
        return 0;
    }
    double (*func)(double) = dlsym(lib_handle, data->func_name);
    data->result = func(data->value);
    sem_post(&(data->response_ready));
    return 1;
}

int main(int argc, char *argv[]) {
    const char *lib_name = argv[1];
    void *lib_handle = dlopen(lib_name, RTLD_NOW);
    if (!lib_handle) {
        fprintf(stderr, "dlopen: %s\n", dlerror());
        return 1;
    }

    const char *shared_memory_name = "/shared_data_t12.15.22.5";
    int shm_fd = shm_open(shared_memory_name, O_RDWR | O_CREAT | O_EXCL, 0664);
    ftruncate(shm_fd, sizeof(shared_data_t));
    shared_data_t *shared_data = mmap(NULL, sizeof(shared_data_t),
                                      PROT_WRITE | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);

    sem_init(&(shared_data->request_ready), 1, 0);
    sem_init(&(shared_data->response_ready), 1, 0);

    printf("%s\n", shared_memory_name);
    fflush(stdout);

    while (process_request(shared_data, lib_handle) == 1)
        ;

    dlclose(lib_handle);
    shm_unlink(shared_memory_name);
    sem_destroy(&(shared_data->request_ready));
    sem_destroy(&(shared_data->response_ready));
    munmap(shared_data, sizeof(shared_data_t));

    return 0;
}

