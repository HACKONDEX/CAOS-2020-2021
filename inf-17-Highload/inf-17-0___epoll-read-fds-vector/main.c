#include <fcntl.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>

static void prepare_epoll(size_t N, int* fd, int epoll_fd) {
    for (int i = 0; i < N; ++i) {
        fcntl(fd[i], F_SETFL, fcntl(fd[i], F_GETFL) | O_NONBLOCK);
        struct epoll_event event = {
            .events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET,
            .data = {.u32 = i}
        };
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd[i], &event);
    }
}

extern size_t read_data_and_count(size_t N, int in[N]) {
    size_t answer = 0;
    int epoll_fd = epoll_create1(0);
    prepare_epoll(N, in, epoll_fd);

    int not_closed = N;
    while (not_closed > 0) {
        struct epoll_event event;
        int epoll_ret = epoll_wait(epoll_fd, &event, 1, 1000);
        if (epoll_ret <= 0) {
            continue;
        }
        int i = event.data.u32;

        char buf[100];
        int read_bytes = 0;
        while ((read_bytes = read(in[i], buf, sizeof(buf))) > 0) {
            answer += read_bytes;
        }
        if (read_bytes == 0) {
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, in[i], NULL);
            close(in[i]);
            in[i] = -1;
            not_closed -= 1;
        }
    }
    close(epoll_fd);

    return answer;
}

