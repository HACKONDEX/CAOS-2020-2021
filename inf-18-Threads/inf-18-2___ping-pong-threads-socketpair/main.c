#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

int fd[2][2];
const int FINISH_CODE = 200;

static void* first_thread_func(int* arg) {
    int number = * arg;
    while (true) {
        number -= 3;
        printf("%d\n", number);
        write(fd[0][1], &number, sizeof(number));
        read(fd[1][0], &number, sizeof(number));
        if(number == FINISH_CODE) {
            break;
        } else if(number == 0 || number > 100) {
            write(fd[0][1], &FINISH_CODE, sizeof(FINISH_CODE));
            break;
        }
    }
    return NULL;
}
static void* second_thread_func(void* arg) {
    int number = 0;
    while (true) {
        read(fd[0][0], &number, sizeof(number));
        if(number == FINISH_CODE) {
            break;
        } else if(number == 0 || number > 100) {
            write(fd[1][1], &FINISH_CODE, sizeof(FINISH_CODE));
            break;
        }
        number += 5;
        printf("%d\n", number);
        write(fd[1][1], &number, sizeof(number));
    }
    return NULL;
}

int main(int argc, char* argv[]) {

    int initial_number = strtoll(argv[1], NULL, 10);
    socketpair(AF_UNIX, SOCK_STREAM, 0, fd[0]);
    socketpair(AF_UNIX, SOCK_STREAM, 0, fd[1]);
    pthread_t threads[2];
    pthread_create(&threads[0], NULL, (void* (*)(void*))first_thread_func, (void *)&initial_number);
    pthread_create(&threads[1], NULL, second_thread_func, 0);

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    for(int i = 0; i <=1; ++i) {
        for(int j = 0; j <= 1; ++j) {
            shutdown(fd[i][j], SHUT_RDWR);
            close(fd[i][j]);
        }
    }

    return 0;
}

