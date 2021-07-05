#define GNU_SOURCE

#include <assert.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXSIZE 4096

char *form_package(char *file_name, const char *server_name, char *path) {

    int fd = open(file_name, O_RDWR);
    size_t content_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    char *content = calloc(sizeof(char), content_size);
    read(fd, content, content_size);
    close(fd);

    size_t package_max_size = MAXSIZE + content_size;
    char *package = calloc(sizeof(char), package_max_size);

    snprintf(package, package_max_size,
             "POST %s HTTP/1.1\nHost: %s\nConnection: close\nContent-Length: %zu\n\n%s\n\n",
             path, server_name, content_size, content);

    free(content);
    return package;
}

void post_request(char *server_name, char *full_path, char *file_name) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    struct addrinfo *result;
    if (getaddrinfo(server_name, "http", &hints, &result) != 0) {
        assert(0);
    }

    int connection_socket = 0;
    struct addrinfo *rp;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        connection_socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (connection_socket == -1)
            continue;
        if (connect(connection_socket, rp->ai_addr, rp->ai_addrlen) != -1) {
            break;
        }
        close(connection_socket);
    }

    if (connection_socket == -1) {
        assert(0);
    }

    char *package = form_package(file_name, server_name, full_path);
    size_t package_size = strlen(package);
    write(connection_socket, package, package_size);
    free(package);

    int fd_clone = dup(connection_socket);
    FILE *received_file = fdopen(fd_clone, "r");
    if (received_file == NULL) {
        assert(0);
    }

    char *buffer = calloc(sizeof(char), package_size);
    while (fgets(buffer, package_size, received_file) > 0) {
        size_t buf_size = strlen(buffer);
        if (buf_size == 2 || buf_size == 1) {
            while (fgets(buffer, package_size, received_file) > 0) {
                write(1, buffer, strlen(buffer));
            }

            break;
        }
    }
    free(buffer);
    fclose(received_file);
    shutdown(connection_socket, SHUT_RDWR);
    close(connection_socket);
}

int main(int argc, char *argv[]) {
    char *server_name = argv[1];
    char *full_path = argv[2];
    char *file_name = argv[3];

    post_request(server_name, full_path, file_name);
    return 0;
}

