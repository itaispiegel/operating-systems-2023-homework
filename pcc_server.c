#include <arpa/inet.h>
#include <netinet/ip.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1048576
#define PRINTABLE_CHARACTERS_START 0x20
#define PRINTABLE_CHARACTERS_END 0x7e

static bool is_done;
static bool is_handling_client;
static int pcc_total[PRINTABLE_CHARACTERS_END - PRINTABLE_CHARACTERS_START + 1];

struct arguments {
    unsigned short port;
};

void usage() {
    printf("./pcc_server port\n");
}

int parse_arguments(int argc, char *argv[], struct arguments *args) {
    if (argc != 2) {
        usage();
        return EXIT_FAILURE;
    }
    args->port = atoi(argv[1]);
    return EXIT_SUCCESS;
}

void show_total() {
    uint8_t i;
    for (i = PRINTABLE_CHARACTERS_START; i <= PRINTABLE_CHARACTERS_END; i++) {
        printf("char '%c' : %u times\n", i,
               pcc_total[i - PRINTABLE_CHARACTERS_START]);
    }
    exit(0);
}

void sigint_handler(int signum) {
    is_done = true;
    if (!is_handling_client) {
        show_total();
    }
}

unsigned int pcc(char *str, unsigned int len) {
    unsigned int pcc = 0;
    unsigned int i;

    for (i = 0; i < len; i++) {
        if (str[i] >= PRINTABLE_CHARACTERS_START &&
            str[i] <= PRINTABLE_CHARACTERS_END) {
            pcc++;
            pcc_total[str[i] - PRINTABLE_CHARACTERS_START]++;
        }
    }
    return pcc;
}

int main(int argc, char *argv[]) {
    struct arguments args;
    int res, s, peer_sock = -1;
    struct sockaddr_in my_addr, peer_addr;
    socklen_t peer_addr_len;
    unsigned int N;
    int enable = 1;
    char *buff;
    unsigned int recvd_bytes, file_pcc, buff_total_size, buff_curr_size,
        file_pcc_nl, read_bytes;

    if ((res = parse_arguments(argc, argv, &args)) != EXIT_SUCCESS) {
        return res;
    }

    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal");
        return 1;
    }

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt");
        return EXIT_FAILURE;
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(args.port);

    if ((bind(s, (const struct sockaddr *)&my_addr, sizeof(my_addr))) != 0) {
        perror("bind");
        return EXIT_FAILURE;
    }

    if ((listen(s, 10)) != 0) {
        perror("listen");
        return EXIT_FAILURE;
    }

    while (!is_done) {
        is_handling_client = false;
        if ((peer_sock = accept(s, (struct sockaddr *)&peer_addr,
                                &peer_addr_len)) < 0) {
            perror("accept");
            return EXIT_FAILURE;
        }

        is_handling_client = true;
        read(peer_sock, &N, 4);
        N = ntohl(N);

        recvd_bytes = 0, file_pcc = 0;
        while (recvd_bytes < N) {
            buff_total_size = MIN(N - recvd_bytes, MAX_BUFFER_SIZE);
            buff_curr_size = 0;
            buff = malloc(buff_total_size);
            while (buff_curr_size < buff_total_size) {
                if ((read_bytes = read(peer_sock, buff + buff_curr_size,
                                       buff_total_size - buff_curr_size)) < 0) {
                    perror("read");
                    return EXIT_FAILURE;
                }
                buff_curr_size += read_bytes;
            }
            file_pcc += pcc(buff, buff_total_size);
            recvd_bytes += buff_curr_size;
            free(buff);
        }

        file_pcc_nl = htonl(file_pcc);
        if (write(peer_sock, &file_pcc_nl, 4) < 0) {
            perror("write");
        }
        close(peer_sock);
    }

    close(s);
    show_total();
}
