#include <assert.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXSIZE 8192

void add_to_str(char *buffer, size_t *size, char *str) {
    size_t length = strlen(str);
    for (size_t i = 0; i < length; ++i) {
        buffer[(*size)++] = str[i];
    }
}

size_t form_package(char *buffer, char *server_name, char *server_file_path) {

    size_t size = 0;
    char *header_first_part = "GET ";
    add_to_str(buffer, &size, header_first_part);

    add_to_str(buffer, &size, server_file_path);
    char *header_second_part = " HTTP/1.1\nHost: ";
    add_to_str(buffer, &size, header_second_part);
    add_to_str(buffer, &size, server_name);

    char *header_third_part = "\nConnection: close\n\n";
    add_to_str(buffer, &size, header_third_part);
    buffer[++size] = '\0';
    return size;
}

void get_request(char *server_name, char *server_file_path) {
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

    char package[MAXSIZE];
    size_t package_size = form_package(package, server_name, server_file_path);

    write(connection_socket, package, package_size);

    int fd_clone = dup(connection_socket);
    FILE *received_file = fdopen(fd_clone, "r");
    char buffer[MAXSIZE];
    while (fgets(buffer, sizeof(buffer), received_file) > 0) {
        size_t buf_size = strlen(buffer);
        if (buf_size == 2 || buf_size == 1) {
            while (fgets(buffer, sizeof(buffer), received_file) > 0) {
                write(1, buffer, strlen(buffer));
            }
            break;
        }
    }
    fclose(received_file);
    shutdown(connection_socket, SHUT_RDWR);
    close(connection_socket);
}

int main(int argc, char *argv[]) {
    char *server_name = argv[1];
    char *server_file_path = argv[2];
    get_request(server_name, server_file_path);
    return 0;
}

