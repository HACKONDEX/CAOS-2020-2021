#define _GNU_SOURCE
#include <inttypes.h>
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

struct Item {
    int64_t key;
    _Atomic(struct Item *) next;
};

struct ThreadData {
    int32_t index;
};

int32_t threads_count = 0;
int32_t elements_count = 0;
const int MY_STACK_SIZE = 16384;
_Atomic(struct Item *) list_head;
struct Item *linked_list = NULL;

static void *thread_function(struct ThreadData *info) {
    for (int i = 0; i < elements_count; ++i) {
        int array_index = elements_count * info->index + i;
        if (array_index == 0) {
            continue;
        }
        linked_list[array_index].key = array_index;
        _Atomic(struct Item *) current_element = linked_list + array_index;
        while (1) {
            atomic_store(&current_element->next, list_head->next);
            if (atomic_compare_exchange_strong(&list_head->next, &current_element->next, current_element)) {
                break;
            }
            sched_yield();
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    threads_count = strtol(argv[1], NULL, 10);
    elements_count = strtol(argv[2], NULL, 10);

    pthread_t *thread_array = calloc(threads_count + 1, sizeof(pthread_t));
    struct ThreadData *thread_info = calloc(threads_count, sizeof(struct ThreadData));
    int max_count = threads_count * elements_count;
    linked_list = calloc(max_count, sizeof(struct Item));
    for (int i = 0; i < max_count; ++i) {
        linked_list[i].next = NULL;
    }
    list_head = linked_list;
    list_head->key = 0;

    for (int i = 0; i < threads_count; ++i) {
        thread_info[i].index = i;
        pthread_attr_t thread_attr;
        pthread_attr_init(&thread_attr);
        pthread_attr_setstacksize(&thread_attr, MY_STACK_SIZE);
        pthread_create(thread_array + i, &thread_attr, (void *(*) (void *) ) thread_function,
                       (void *) (thread_info + i));
        pthread_attr_destroy(&thread_attr);
    }

    for (int i = 0; i < threads_count; ++i) {
        pthread_join(thread_array[i], NULL);
    }

    while (1) {
        printf("%" PRId64 "\n", list_head->key);
        if (list_head->next == NULL) { break; }
        list_head = list_head->next;
    }

    free(linked_list);
    free(thread_info);
    free(thread_array);
    return 0;
}

