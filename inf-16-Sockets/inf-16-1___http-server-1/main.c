#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
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

static void sigterm_sigint_handler(int signum) {
    write(stop_pipe_fds[1], "X", 1);
    // Самая первая запись одного символа в пайп пройдет всегда успешно, так как буффер пуст.
}

void set_handlers() {
    sigaction(SIGTERM,
              &(struct sigaction){
                  .sa_handler = sigterm_sigint_handler,
                  .sa_flags = SA_RESTART},
              NULL);
    sigaction(SIGINT,
              &(struct sigaction){
                  .sa_handler = sigterm_sigint_handler,
                  .sa_flags = SA_RESTART},
              NULL);
}

const int MAXSIZE = 4096;
const char *OK_STR = "HTTP/1.1 200 OK\r\n";
const char *FORBIDDEN_STR = "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\n\r\n";
const char *NOT_FOUND_STR = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
int OK_SIZE = 0;
int FORBIDDEN_SIZE = 0;
int NOT_FOUND_SIZE = 0;

void process_client(int directory_fd, int connection_fd) {
    char file_name[MAXSIZE];

    int stream_fd = dup(connection_fd);
    FILE *connection_stream = fdopen(stream_fd, "r");
    fscanf(connection_stream, "GET %s HTTP/1.1", file_name);

    char tmp_buffer[MAXSIZE];
    while (fgets(tmp_buffer, MAXSIZE, connection_stream) != NULL) {
        if (tmp_buffer[0] == '\r' && tmp_buffer[1] == '\n') {
            break;
        }
    }

    fclose(connection_stream);

    if (faccessat(directory_fd, file_name, F_OK, 0) != 0) {
        write(connection_fd, NOT_FOUND_STR, NOT_FOUND_SIZE);
        return;
    }

    if (faccessat(directory_fd, file_name, R_OK, 0) != 0) {
        write(connection_fd, FORBIDDEN_STR, FORBIDDEN_SIZE);
        return;
    }

    // Send OK
    write(connection_fd, OK_STR, OK_SIZE);

    // Get and send FileSize
    int file_fd = openat(directory_fd, file_name, O_RDONLY);
    long int file_size = lseek(file_fd, 0, SEEK_END);
    lseek(file_fd, 0, SEEK_SET);
    char buffer_[256];
    snprintf(buffer_, 256, "Content-Length: %ld\r\n\r\n", file_size);
    write(connection_fd, buffer_, strlen(buffer_));

    // Send the file
    int send_ret = sendfile(connection_fd, file_fd, NULL, file_size);
    conditional_handle_error(send_ret == -1, " Couldn't copy file content");

    close(file_fd);
}

const int LISTEN_BACKLOG = 4096;

void server_job(const char *directory_path, int socket_fd, int stop_fd, int epoll_fd) {

    OK_SIZE = strlen(OK_STR);
    FORBIDDEN_SIZE = strlen(FORBIDDEN_STR);
    NOT_FOUND_SIZE = strlen(NOT_FOUND_STR);

    int directory_fd = open(directory_path, O_RDONLY);

    while (true) {
        struct epoll_event event;
        // Читаем события из epoll-объект (то есть из множества файловых дескриптотров, по которым есть события)
        int epoll_ret = epoll_wait(epoll_fd, &event, 1, 1000);
        if (epoll_ret <= 0) {
            continue;
        }

        // Если пришло событие из stop_fd - пора останавливаться
        if (event.data.fd == stop_fd) {
            break;
        }

        /* Иначе пришло событие из socket_fd и accept
        * отработает мгновенно, так как уже подождали в epoll */

        struct sockaddr_in peer_addr = {0};
        socklen_t peer_addr_size = sizeof(struct sockaddr_in);
        // Принимаем соединение и записываем адрес
        int connection_fd = accept(socket_fd, (struct sockaddr *) &peer_addr, &peer_addr_size);
        conditional_handle_error(connection_fd == -1, "can't accept incoming connection");

        //Process client
        process_client(directory_fd, connection_fd);

        shutdown(connection_fd, SHUT_RDWR);
        close(connection_fd);
    }

    close(directory_fd);
}

void launch_server(const int PORT, const char *directory_path, int stop_fd) {

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    conditional_handle_error(socket_fd == -1, "can't initialize socket");

    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr = 0};

    int bind_ret = bind(socket_fd, (struct sockaddr *) &address, sizeof(address));// Привязали сокет к порту
    conditional_handle_error(bind_ret == -1, "can't bind to unix socket");

    // Говорим что готовы принимать соединения. Не больше чем LISTEN_BACKLOG за раз
    int listen_ret = listen(socket_fd, LISTEN_BACKLOG);
    conditional_handle_error(listen_ret == -1, "can't listen to unix socket");

    int epoll_fd = epoll_create1(0);
    {
        int fds[] = {stop_fd, socket_fd, -1};
        for (int *fd = fds; *fd != -1; ++fd) {
            struct epoll_event event = {
                .events = EPOLLIN | EPOLLERR | EPOLLHUP,
                .data = {.fd = *fd}};
            epoll_ctl(epoll_fd, EPOLL_CTL_ADD, *fd, &event);
        }
    }

    server_job(directory_path, socket_fd, stop_fd, epoll_fd);

    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
}

int main(int argc, char *argv[]) {

    const int PORT = (int) strtol(argv[1], NULL, 10);

    pipe(stop_pipe_fds);
    set_handlers();

    launch_server(PORT, argv[2], stop_pipe_fds[0]);

    close(stop_pipe_fds[0]);
    close(stop_pipe_fds[1]);
    return 0;
}

