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
#define PIPE "|"
#define PIPE_READ_END 0
#define PIPE_WRITE_END 1

void sigchld_handler(int signum) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) {
    }
    errno = saved_errno;
}

int index_of_pipe(char **arglist) {
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], PIPE) == 0) {
            return i;
        }
    }
    return -1;
}

int run_pipeline(char **arglist, int pipe_index) {
    pid_t pid1, pid2, wpid;
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return false;
    }

    pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        return false;
    } else if (pid1 == 0) {
        signal(SIGINT, SIG_DFL);
        close(pipefd[PIPE_READ_END]);
        arglist[pipe_index] = NULL;
        if (dup2(pipefd[PIPE_WRITE_END], STDOUT_FILENO) < 0) {
            perror("dup2");
            return false;
        }
        close(pipefd[PIPE_WRITE_END]);
        execvp(arglist[0], arglist);
        perror("execvp");
        return false;
    }

    pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        return false;
    } else if (pid2 == 0) {
        signal(SIGINT, SIG_DFL);
        close(pipefd[PIPE_WRITE_END]);
        if (dup2(pipefd[PIPE_READ_END], STDIN_FILENO) < 0) {
            perror("dup2");
            return false;
        }
        close(pipefd[PIPE_READ_END]);
        execvp(arglist[pipe_index + 1], arglist + pipe_index + 1);
        perror("execvp");
        return false;
    }

    close(pipefd[PIPE_READ_END]);
    close(pipefd[PIPE_WRITE_END]);
    wpid = waitpid(pid1, NULL, WUNTRACED);
    if (wpid < 0 && errno != ECHILD && errno != EINTR) {
        perror("waitpid");
        return false;
    }

    wpid = waitpid(pid2, NULL, WUNTRACED);
    if (wpid < 0 && errno != ECHILD && errno != EINTR) {
        perror("waitpid");
        return false;
    }

    return true;
}

int prepare(void) {
    struct sigaction zombie_reaper, ignore_ctrl_c;
    zombie_reaper.sa_handler = &sigchld_handler;
    zombie_reaper.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    ignore_ctrl_c.sa_handler = SIG_IGN;
    ignore_ctrl_c.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &zombie_reaper, NULL) != 0) {
        perror("sigaction(SIGCHLD)");
        return -1;
    }
    if (sigaction(SIGINT, &ignore_ctrl_c, NULL) != 0) {
        perror("sigaction(SIGINT)");
        return -1;
    }
    return 0;
}

int process_arglist(int count, char **arglist) {
    bool run_in_foreground = strcmp(arglist[count - 1], AMPERSAND) != 0;
    int pipe_index = index_of_pipe(arglist);

    if (pipe_index != -1) {
        return run_pipeline(arglist, pipe_index);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return false;
    } else if (pid == 0) {
        if (run_in_foreground) {
            signal(SIGINT, SIG_DFL);
        } else {
            arglist[count - 1] = NULL;
        }
        execvp(arglist[0], arglist);
        perror("execvp");
        return false;
    }

    if (run_in_foreground) {
        pid_t wpid = waitpid(pid, NULL, WUNTRACED);
        if (wpid < 0 && errno != ECHILD && errno != EINTR) {
            perror("waitpid");
            return false;
        }
    }

    return true;
}

int finalize(void) { return 0; }
