#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "message_slot.h"

struct arguments {
    char *message_slot_path;
    int channel_id;
};

void print_usage() {
    fprintf(stderr,
            "Usage: message_reader <message_slot_file_path> <channel_id>\n");
    exit(1);
}

void parse_arguments(struct arguments *args, int argc, char *argv[]) {
    if (argc != 3) {
        print_usage();
    }

    args->message_slot_path = argv[1];
    args->channel_id = atoi(argv[2]);
}

int main(int argc, char *argv[]) {
    int fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    struct arguments *args =
        (struct arguments *)malloc(sizeof(struct arguments));
    parse_arguments(args, argc, argv);

    fd = open(args->message_slot_path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return errno;
    }

    if (ioctl(fd, MSG_SLOT_CHANNEL, args->channel_id) < 0) {
        perror("ioctl");
        return errno;
    }

    bytes_read = read(fd, buffer, BUFFER_SIZE);
    if (bytes_read < 0) {
        perror("read");
        return errno;
    }

    close(fd);
    write(STDOUT_FILENO, buffer, bytes_read);
    return 0;
}
