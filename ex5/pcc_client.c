#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1048576

struct arguments {
    char *ip;
    unsigned short port;
    char *fpath;
};

void usage() {
    printf("./pcc_client ip port fpath\n");
}

int parse_arguments(int argc, char *argv[], struct arguments *args) {
    if (argc != 4) {
        usage();
        return EXIT_FAILURE;
    }

    args->ip = argv[1];
    args->port = atoi(argv[2]);
    args->fpath = argv[3];
    return EXIT_SUCCESS;
}

unsigned int file_size(int fd) {
    unsigned int fsize;
    if ((fsize = lseek(fd, 0, SEEK_END)) < 0) {
        perror("lseek");
        exit(1);
    }

    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("lseek");
        exit(1);
    }
    return fsize;
}

int main(int argc, char *argv[]) {
    struct arguments args;
    int exit_code, fd, server_sock;
    struct sockaddr_in addr;
    unsigned int N, N_nl, pcc_nl, buffer_size, sent_bytes_cnt = 0;
    char *buffer;

    if ((exit_code = parse_arguments(argc, argv, &args)) != EXIT_SUCCESS) {
        return exit_code;
    }

    if ((fd = open(args.fpath, O_RDONLY)) < 0) {
        perror("open");
        return EXIT_FAILURE;
    }

    N = file_size(fd);
    addr = (struct sockaddr_in){.sin_family = AF_INET,
                                .sin_port = htons(args.port),
                                .sin_addr.s_addr = inet_addr(args.ip)};
    if (inet_pton(AF_INET, args.ip, &addr.sin_addr.s_addr) <= 0) {
        perror("inet_pton");
    }

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    if (connect(server_sock, (const struct sockaddr *)&addr, sizeof(addr)) !=
        0) {
        perror("connect");
        return EXIT_FAILURE;
    }

    N_nl = htonl(N);
    write(server_sock, &N_nl, 4);

    while (sent_bytes_cnt < N) {
        buffer_size = MIN(MAX_BUFFER_SIZE, N);
        buffer = malloc(buffer_size);
        read(fd, buffer, buffer_size);
        write(server_sock, buffer, buffer_size);
        sent_bytes_cnt += buffer_size;
        free(buffer);
    }

    if (read(server_sock, &pcc_nl, 4) < 0) {
        perror("read");
    }

    printf("# of printable characters: %u\n", ntohl(pcc_nl));

    close(server_sock);
    close(fd);
    return EXIT_SUCCESS;
}
