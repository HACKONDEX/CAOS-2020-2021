#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

struct ThreadData {
    int iterations_count;
    int threads_count;
    int index;
    double *numbers_array;
    pthread_mutex_t *mutex_array;
};

#define Left(i, n) (i == 0 ? n - 1 : i - 1)
#define Right(i, n) (i == n - 1 ? 0 : i + 1)

static void *thread_function(struct ThreadData *info) {
    int iterations_count = info->iterations_count;
    int threads_count = info->threads_count;
    int indexes[3];
    int index = indexes[0] = info->index;
    double *numbers_array = info->numbers_array;
    pthread_mutex_t *mutex_array = info->mutex_array;
    int order[3] = {0, 1, 2};
    for (int i = 0; i < iterations_count; ++i) {
        int left = indexes[1] = Left(index, threads_count);
        int right = indexes[2] = Right(index, threads_count);
        for(int j = 0; j < 3; ++j) {
            order[j] = (j +(index % 6)) % 3;
        }
        for(int j = 0; j < 3; ++j) {
            pthread_mutex_lock(mutex_array + indexes[order[j]]);
        }

        numbers_array[index] += 1.0;
        numbers_array[left] += 0.99;
        numbers_array[right] += 1.01;

        pthread_mutex_unlock(&mutex_array[right]);
        pthread_mutex_unlock(&mutex_array[left]);
        pthread_mutex_unlock(&mutex_array[index]);
    }
}

int main(int argc, char *argv[]) {
    int iterations_count = strtol(argv[1], NULL, 10);
    int threads_count = strtol(argv[2], NULL, 10);

    double *numbers = calloc(threads_count, sizeof(double));
    pthread_t *thread_array = calloc(threads_count, sizeof(pthread_t));
    struct ThreadData *thread_info = calloc(threads_count, sizeof(struct ThreadData));
    pthread_mutex_t *mutex_array = calloc(threads_count, sizeof(pthread_mutex_t));

    const int MY_STACK_SIZE = 16384;
    for (int i = 0; i < threads_count; ++i) {
        pthread_mutex_init(mutex_array + i, NULL);
        thread_info[i].index = i;
        thread_info[i].iterations_count = iterations_count;
        thread_info[i].threads_count = threads_count;
        thread_info[i].numbers_array = numbers;
        thread_info[i].mutex_array = mutex_array;

        pthread_attr_t thread_attr;
        pthread_attr_init(&thread_attr);
        pthread_attr_setstacksize(&thread_attr, MY_STACK_SIZE);
        pthread_create(thread_array + i, &thread_attr, (void *(*) (void *) ) thread_function,
                       (void *) (thread_info + i));
    }

    for (int i = 0; i < threads_count; ++i) {
        pthread_join(thread_array[i], NULL);
    }

    for (int i = 0; i < threads_count; ++i) {
        printf("%.10g\n", numbers[i]);
    }


    free(mutex_array);
    free(thread_info);
    free(thread_array);
    free(numbers);
    return 0;
}
