#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define BUFFER_SIZE 4096
char buffer[BUFFER_SIZE];

int pipe_fd[2];

enum PID {
    PID_PARENT,
    PID_CHILD,
    PID_FAIL,
};

enum PID check(pid_t pid) {
    if (pid == 0) {
        return PID_CHILD;
    } else if (pid > 0) {
        return PID_PARENT;
    }
    return PID_FAIL;
}

void parent(pid_t pid) {
    printf("Parent process waiting for output from child...\n");

    // Close the write end of the pipe in the parent
    close(pipe_fd[1]);

    // Read from the pipe until EOF
    printf("\nOutput from child:\n");
    printf("------------------\n");

    ssize_t nof_bytes_read;

    while ((nof_bytes_read = read(pipe_fd[0], buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[nof_bytes_read] = '\0';  // Null-terminate the string
        printf("%s", buffer);
    }

    printf("------------------\n");

    if (nof_bytes_read == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    // Close the read end of the pipe
    close(pipe_fd[0]);

    // Wait for theprograma child to terminate
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        printf("\nChild process exited with status %d\n", WEXITSTATUS(status));
    } else {
        printf("\nChild process did not exit normally\n");
    }
}

void child(){
    // Child process
    printf("Child process executing 'ls -l'...\n");

    // Close the read end of the pipe in the child
    close(pipe_fd[0]);

    // Redirect stdout to the write end of the pipe
    if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
        perror("dup2");
        exit(EXIT_FAILURE);
    }

    // Close the write end of the pipe as it's been duplicated
    close(pipe_fd[1]);

    // Execute the command
    execlp("ls", "ls", "-l", NULL);

    // If execlp returns, it must have failed
    perror("execlp");
    exit(EXIT_FAILURE);
}

int main() {
    if (pipe(pipe_fd) == -1){
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    switch (check(pid)) {
        case PID_FAIL:
            perror("fork");
            exit(EXIT_FAILURE);
            break;
        case PID_PARENT:
            parent(pid);
            break;
        case PID_CHILD:
            child();
            break;
        default:
            exit(1);
    }

    return 0;
}
