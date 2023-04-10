#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define AMPERSAND "&"

void sigchld_handler(int signum) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) {
    }
    errno = saved_errno;
}

int prepare(void) {
    struct sigaction zombie_reaper;
    zombie_reaper.sa_handler = &sigchld_handler;
    zombie_reaper.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &zombie_reaper, NULL) != 0) {
        perror("sigaction(SIGCHLD)");
        return -1;
    }

    struct sigaction ignore_ctrl_c;
    ignore_ctrl_c.sa_handler = SIG_IGN;
    ignore_ctrl_c.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &ignore_ctrl_c, NULL) != 0) {
        perror("sigaction(SIGINT)");
        return -1;
    }

    return 0;
}

int process_arglist(int count, char **arglist) {
    int has_ampersand = strcmp(arglist[count - 1], AMPERSAND);

    if (has_ampersand == 0) {
        arglist[count - 1] = NULL;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return false;
    } else if (pid == 0) {
        execvp(arglist[0], arglist);
        perror("execvp");
        return false;
    }

    if (has_ampersand != 0) {
        pid_t wpid = waitpid(pid, NULL, WUNTRACED);
        if (wpid < 0 && errno != ECHILD && errno != EINTR) {
            perror("waitpid");
            return false;
        }
    }

    return true;
}

int finalize(void) { return 0; }
