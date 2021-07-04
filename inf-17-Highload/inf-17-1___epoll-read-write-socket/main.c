#include <ctype.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define conditional_handle_error(stmt, msg) \
    do {                                    \
        if (stmt) {                         \
            perror(msg " (" #stmt ")");     \
            exit(EXIT_FAILURE);             \
        }                                   \
    } while (0)

static int stop_pipe_fds[2] = {-1, -1};

static void sigterm_handler(int signum) {
    write(stop_pipe_fds[1], "X", 1);
}

void set_handlers() {
    sigaction(SIGTERM,
              &(struct sigaction){
                  .sa_handler = sigterm_handler,
                  .sa_flags = SA_RESTART},
              NULL);
}

const int MAXSIZE = 4096;
int all_fds[4096];

void process_client(int connection_fd) {
    char buffer[MAXSIZE];
    int read_bytes_count = 0;
    while ((read_bytes_count = read(connection_fd, buffer, MAXSIZE)) > 0) {
        for (int i = 0; i < read_bytes_count; ++i) {
            buffer[i] = toupper(buffer[i]);
        }
        write(connection_fd, buffer, read_bytes_count);
    }

    all_fds[connection_fd] = -1;
    shutdown(connection_fd, SHUT_RDWR);
    close(connection_fd);
}

void add_fd_to_epoll(int epoll_fd, int fd) {
    struct epoll_event event = {
        .events = EPOLLIN | EPOLLERR | EPOLLHUP,
        .data = {.fd = fd}};
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
}

const int LISTEN_BACKLOG = 4096;

void server_job(int socket_fd, int stop_fd, int epoll_fd) {
    while (true) {
        struct epoll_event event;
        int epoll_ret = epoll_wait(epoll_fd, &event, 1, 1000);
        if (epoll_ret <= 0) {
            continue;
        }

        int fd = event.data.fd;

        if (fd == stop_fd) {
            for(int i = 0; i < MAXSIZE; ++i) {
                if(all_fds[i] > 0) {
                    shutdown(i, SHUT_RDWR);
                    close(i);
                }
            }
            break;
        } else if (fd == socket_fd) {
            struct sockaddr_in peer_addr = {0};
            socklen_t peer_addr_size = sizeof(struct sockaddr_in);
            int connection_fd = accept(socket_fd, (struct sockaddr *) &peer_addr, &peer_addr_size);
            conditional_handle_error(connection_fd == -1, "can't accept incoming connection");

            fcntl(connection_fd, F_SETFL, fcntl(connection_fd, F_GETFL) | O_NONBLOCK);
            all_fds[connection_fd] = connection_fd;
            add_fd_to_epoll(epoll_fd, connection_fd);
        } else {
            process_client(fd);
        }
    }
}

void launch_server(const int PORT, int stop_fd) {

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    conditional_handle_error(socket_fd == -1, "can't initialize socket");

    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr = 0};

    int bind_ret = bind(socket_fd, (struct sockaddr *) &address, sizeof(address));
    conditional_handle_error(bind_ret == -1, "can't bind to unix socket");

    int listen_ret = listen(socket_fd, LISTEN_BACKLOG);
    conditional_handle_error(listen_ret == -1, "can't listen to unix socket");

    int epoll_fd = epoll_create1(0);
    {
        int fds[] = {stop_fd, socket_fd, -1};
        for (int *fd = fds; *fd != -1; ++fd) {
            add_fd_to_epoll(epoll_fd, *fd);
        }
    }

    server_job(socket_fd, stop_fd, epoll_fd);

    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
}

int main(int argc, char *argv[]) {
    const int PORT = (int) strtol(argv[1], NULL, 10);

    pipe(stop_pipe_fds);
    set_handlers();

    launch_server(PORT, stop_pipe_fds[0]);

    close(stop_pipe_fds[0]);
    close(stop_pipe_fds[1]);
    return 0;
}

