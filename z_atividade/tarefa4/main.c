#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <wait.h>

extern char **environ;

enum PID {
    PARENT,
    CHILD,
    FAIL,
};

enum PID check_pid(pid_t pid) {
    if (pid == 0) {
        return CHILD;
    } else if (pid > 0) {
        return PARENT;
    }
    return FAIL;
}

void shell(char cmd[], int *fds) {
    int duto = pipe(fds);

    pid_t pid = fork();

    char *argv[] = {
        "/usr/bin/sh",
        "-c",
        cmd,
        NULL
    };

    switch (check_pid(pid)) {
        case CHILD:
            execve("/usr/bin/sh", argv, environ);
            exit(1);
        break;

        case PARENT:
            wait(&pid);
        break;

        case FAIL:

        break;
    }
}

int main() {
    int fd_pipe[2] = {1,1};

    shell("ls -la", fd_pipe);

    return 0;
}
