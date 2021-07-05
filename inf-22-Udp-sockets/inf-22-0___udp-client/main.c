#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int port = (int) strtol(argv[1], NULL, 10);

    int udp_sending_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct hostent *hp = gethostbyname("localhost");

    struct sockaddr_in whereto = {.sin_family = hp->h_addrtype, .sin_port = htons(port)};
    memcpy(&whereto.sin_addr, hp->h_addr, hp->h_length);

    int number = 0;
    while (scanf("%d", &number) > 0) {
        sendto(udp_sending_sock, &number, sizeof(number), 0,
               (struct sockaddr *) &whereto, sizeof(struct sockaddr_in));
        int received_number = 0;
        recvfrom(udp_sending_sock, &received_number, sizeof(received_number),
                 0, NULL, 0);
        printf("%d ", received_number);
    }
    close(udp_sending_sock);
    return 0;
}

