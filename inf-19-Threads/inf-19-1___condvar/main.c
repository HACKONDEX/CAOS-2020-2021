#define _GNU_SOURCE
#include <inttypes.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

struct ThreadData {
    int64_t left;
    int64_t right;
    int64_t *prime_num;
    int32_t numbers_count;
    int *status;
    pthread_mutex_t mutex;
    pthread_cond_t any_prime_num;
};

int is_prime(int64_t num) {
    if (num <= 1) {
        return 0;
    }
    if (num == 2 || num == 3) {
        return 1;
    }
    int return_code = 1;
    double divisor_max = sqrt(num);
    for (int64_t divisor = 2; divisor <= divisor_max; ++divisor) {
        if (num % divisor == 0) {
            return_code = 0;
            break;
        }
    }
    return return_code;
}

static void *thread_function(struct ThreadData *info) {
    int64_t left = info->left;
    int64_t right = info->right;
    int32_t numbers_count = 0;
    for (int64_t num = left; num <= right; ++num) {
        if (is_prime(num) == 1) {
            pthread_mutex_lock(&info->mutex);
            while (*info->status != 0) {
                pthread_cond_wait(&info->any_prime_num, &info->mutex);
            }
            *info->prime_num = num;
            *info->status = 1;
            ++numbers_count;
            pthread_cond_signal(&info->any_prime_num);
            pthread_mutex_unlock(&info->mutex);
        }
        if (numbers_count >= info->numbers_count) {
            break;
        }
    }
    pthread_mutex_lock(&info->mutex);
    *info->status = 2;

    pthread_cond_signal(&info->any_prime_num);
    pthread_mutex_unlock(&info->mutex);
}

int main(int argc, char *argv[]) {
    int64_t left = strtol(argv[1], NULL, 10);
    int64_t right = strtol(argv[2], NULL, 10);
    int32_t max_numbers_count = strtol(argv[3], NULL, 10);

    int status = 0;
    int64_t prime_num = 0;
    struct ThreadData thread_info;
    thread_info.left = left;
    thread_info.right = right;
    thread_info.numbers_count = max_numbers_count;
    thread_info.status = &status;
    thread_info.prime_num = &prime_num;
    pthread_mutex_init(&thread_info.mutex, NULL);
    pthread_cond_init(&thread_info.any_prime_num, NULL);

    pthread_t thread;
    pthread_create(&thread, NULL, (void *(*) (void *) ) thread_function, (void *) (&thread_info));

    for (int i = 0; i < max_numbers_count; ++i) {
        pthread_mutex_lock(&thread_info.mutex);
        while (status == 0) {
            pthread_cond_wait(&thread_info.any_prime_num, &thread_info.mutex);
        }
        if (status == 1) {
            printf("%" PRId64 "\n", prime_num);
            status = 0;
            prime_num = 0;
        } else if (status == 2) {
            if (prime_num != 0) {
                printf("%" PRId64 "\n", prime_num);
            }
            pthread_mutex_unlock(&thread_info.mutex);
            break;
        }
        pthread_cond_signal(&thread_info.any_prime_num);
        pthread_mutex_unlock(&thread_info.mutex);
    }
    pthread_cond_signal(&thread_info.any_prime_num);
    pthread_join(thread, NULL);
    return 0;
}

