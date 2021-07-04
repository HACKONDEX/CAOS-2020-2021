#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

const int MY_STACK_SIZE = 16384;

static void *thread_function(int64_t *arg) {
    int64_t number = 0;
    while (scanf("%ld", &number) > 0) {
        *arg += number;
        fflush(stdout);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int thread_count = strtoll(argv[1], NULL, 10);
    pthread_t *thread_array = calloc(thread_count + 1, sizeof(pthread_t));
    int64_t *sums = calloc(thread_count + 1, sizeof(int64_t));
    memset(sums, 0, (thread_count + 1) * sizeof(int64_t));
    for (int i = 0; i < thread_count; ++i) {
        pthread_attr_t thread_attr;
        pthread_attr_init(&thread_attr);
        pthread_attr_setstacksize(&thread_attr, MY_STACK_SIZE);
        pthread_create(thread_array + i, &thread_attr, (void *(*) (void *) ) thread_function,
                       (void *) (sums + i));
        pthread_attr_destroy(&thread_attr);
    }

    int64_t whole_sum = 0;
    for (int i = 0; i < thread_count; ++i) {
        pthread_join(thread_array[i], NULL);
        whole_sum += sums[i];
    }
    free(thread_array);
    free(sums);

    printf("%ld", whole_sum);
    return 0;
}

