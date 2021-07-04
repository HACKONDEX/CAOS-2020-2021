#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static void *thread_function(void *args);
void create_join_thread(pthread_t *thread);

void create_join_thread(pthread_t *thread) {

    int create_ret = pthread_create(thread, NULL, thread_function, 0);
    if (create_ret != 0) {
        exit(EXIT_FAILURE);
    }
    int join_ret = pthread_join(*thread, NULL);
    if (join_ret != 0) {
        exit(EXIT_FAILURE);
    }
}

static void *thread_function(void *args) {
    int number = 0;
    if (scanf("%d", &number) <= 0) {
        return NULL;
    }
    pthread_t thread;
    create_join_thread(&thread);
    printf("%d ", number);
    return NULL;
}

int main() {
    pthread_t thread;
    create_join_thread(&thread);
    return 0;
}

