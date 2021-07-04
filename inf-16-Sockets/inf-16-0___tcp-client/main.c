#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define conditional_handle_error(stmt, msg) \
    do {                                    \
        if (stmt) {                         \
            perror(msg " (" #stmt ")");     \
            exit(EXIT_FAILURE);             \
        }                                   \
    } while (0)

int socket_fd = 0;

int main(int argc, char *argv[]) {
    sigset_t mask;
    sigaddset(&mask, SIGPIPE);
    sigprocmask(SIG_BLOCK, &mask, NULL);

    const char *IPV4 = argv[1];
    const int PORT = (int) strtol(argv[2], NULL, 10);

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    conditional_handle_error(socket_fd == -1, "can't initialize socket");

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);

    struct hostent *hosts = gethostbyname(IPV4);
    conditional_handle_error(!hosts, "can't get host by name");
    memcpy(&address.sin_addr, hosts->h_addr_list[0], sizeof(address.sin_addr));

    int connected = 0;
    while (connected != 1) {
        int connect_ret = connect(socket_fd, (struct sockaddr *) &address, sizeof(address));
        if (connect_ret != -1) {
            connected = 1;
        }
    }
    // Successfully connected

    int num = 0;
    int read_write_ret = 1;
    while (scanf("%d", &num) > 0) {
        read_write_ret = write(socket_fd, &num, sizeof(num));
        if (read_write_ret == -1) {
            break;
        }
        read_write_ret = read(socket_fd, &num, sizeof(num));
        if (read_write_ret <= 0) {
            break;
        }
        printf("%d ", num);
    }

    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);

    return 0;
}

