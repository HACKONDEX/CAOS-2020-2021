#include <assert.h>
#include <inttypes.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXPACKET 4096
#define PINGPACKETSIZE 64

volatile sig_atomic_t alarmed = 0;

static void alarm_handler(int signum) {
    alarmed = 1;
}

static void set_handler() {
    sigaction(SIGALRM,
              &(struct sigaction){
                  .sa_handler = alarm_handler,
                  .sa_flags = SA_RESTART},
              NULL);
}

uint16_t calculate_checksum(uint16_t *structure, size_t bytes_count) {
    uint32_t check_sum = 0;

    while (bytes_count > 1) {
        check_sum += *(structure++);
        bytes_count -= 2;
    }
    if (bytes_count == 1) {
        check_sum += *(unsigned char *) structure;
    }

    check_sum = (check_sum >> 16) + (check_sum & 0xFFFF);
    check_sum += (check_sum >> 16);

    uint16_t final_check_sum = ~check_sum;
    return final_check_sum;
}

struct ping_package {
    struct icmphdr header;
    char data[PINGPACKETSIZE - sizeof(struct icmphdr)];
};

void form_package(struct ping_package *package) {
    memset(package, 0, sizeof(struct ping_package));
    package->header.type = ICMP_ECHO;
    package->header.un.echo.id = getpid();
    package->header.un.echo.sequence = 1;
    size_t bytes_count = sizeof(package->data) - 1;
    for (size_t i = 0; i < bytes_count; ++i) {
        package->data[i] = '9';
    }
    package->data[bytes_count] = '\0';
    package->header.checksum = calculate_checksum((uint16_t *) (package), sizeof(struct ping_package));
}
int ping(const char *ip, uint64_t interval) {
    struct hostent *hp = gethostbyname(ip);
    struct sockaddr_in whereto = {.sin_family = hp->h_addrtype, .sin_port = htons(0)};
    memcpy(&whereto.sin_addr, hp->h_addr, hp->h_length);

    int icmp_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    struct ping_package package;
    form_package(&package);
    int responses_count = 0;
    while (alarmed != 1) {
        int send_ret = sendto(icmp_socket, &package, sizeof(package),
                              0, (const struct sockaddr *) &whereto, sizeof(whereto));
        u_char packet[MAXPACKET];
        if (send_ret != -1) {
            if (recvfrom(icmp_socket, packet, sizeof(packet), 0, NULL, 0) > 0) {
                ++responses_count;
            }
        }
        usleep(interval);
    }
    close(icmp_socket);
    return responses_count;
}

int main(int argc, char *argv[]) {
    const char *ip_v4 = argv[1];
    uint64_t timeout_seconds = strtoll(argv[2], NULL, 10);
    uint64_t interval_microseconds = strtoll(argv[3], NULL, 10);
    set_handler();
    alarm(timeout_seconds);
    printf("%d\n", ping(ip_v4, interval_microseconds));
    return 0;
}

