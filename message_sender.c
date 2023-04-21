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
    char *message;
};

void print_usage() {
    fprintf(stderr, "Usage: message_sender <message_slot_file_path> "
                    "<channel_id> <message>\n");
    exit(1);
}

void parse_arguments(struct arguments *args, int argc, char *argv[]) {
    if (argc != 4) {
        print_usage();
    }

    args->message_slot_path = argv[1];
    args->channel_id = atoi(argv[2]);
    args->message = argv[3];
}

int main(int argc, char *argv[]) {
    int fd;
    struct arguments *args =
        (struct arguments *)malloc(sizeof(struct arguments));
    parse_arguments(args, argc, argv);

    fd = open(args->message_slot_path, O_WRONLY);
    if (fd < 0) {
        perror("open");
        return errno;
    }

    if (ioctl(fd, MSG_SLOT_CHANNEL, args->channel_id) < 0) {
        perror("ioctl");
        return errno;
    }

    if (write(fd, args->message, strlen(args->message)) < 0) {
        perror("write");
        return errno;
    }

    close(fd);
    return 0;
}
