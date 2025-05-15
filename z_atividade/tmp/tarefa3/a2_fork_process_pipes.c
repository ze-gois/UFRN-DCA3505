#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct Experiment {
    size_t repetitions;
    char description[30];
    unsigned int sleep_parent;
    unsigned int sleep_child;
    unsigned int sleep_post;
    bool do_parent_wait_child;
};

enum FORK_RESULT {
    FORK_FAIL = -1,
    FORK_CHILD = 0,
    FORK_PARENT = 1,
};

enum FORK_RESULT check_fork(pid_t pid){
    if (pid < 0) return FORK_FAIL;
    if (pid > 0) return FORK_PARENT;
    return FORK_CHILD;
}

void print_process(char const d[]) {
    printf("[%s]: (%d,%d)\n", d, getpid(), getppid());
}

void sleep_process(char const d[], unsigned int duration){
    printf("[%s]: (%d,%d) sleep for %d seconds\n", d, getpid(), getppid(), duration);
    sleep(duration);
}

// Pipe for synchronization
int sync_pipe[2];

// Initialize synchronization mechanism
bool init_lock() {
    // Create a pipe for synchronization
    if (pipe(sync_pipe) == -1) {
        perror("pipe");
        return false;
    }
    return true;
}

// Wait for lock to be released
bool wait_for_unlock() {
    char buf;
    // Block until there is something to read from the pipe
    return read(sync_pipe[0], &buf, 1) == 1;
}

// Release the lock
bool release_lock() {
    char buf = 'X';
    // Write a byte to the pipe to signal the other process
    return write(sync_pipe[1], &buf, 1) == 1;
}

void experiment(struct Experiment *e) {
    // Initialize the synchronization mechanism
    if (!init_lock()) {
        fprintf(stderr, "Failed to initialize lock\n");
        return;
    }

    pid_t child_pid = fork();
    char fc[2];

    switch (check_fork(child_pid)) {
        case FORK_FAIL:
            perror("fork");
            break;

        case FORK_CHILD:
            strcpy(fc, "C");
            print_process(fc);
            sleep_process(fc, e->sleep_child);
            
            // Child notifies parent it's done with critical section
            printf("[C] Releasing lock\n");
            release_lock();
            break;

        case FORK_PARENT:
            strcpy(fc, "P");
            print_process(fc);
            sleep_process(fc, e->sleep_parent);
            
            // Parent waits for child to finish critical section
            printf("[P] Waiting for child to release lock\n");
            wait_for_unlock();
            printf("[P] Lock released by child\n");
            break;
    }

    sleep_process(fc, e->sleep_post);
    print_process(fc);

    if (e->do_parent_wait_child && check_fork(child_pid) == FORK_PARENT) {
        int status;
        wait(&status);
        printf("[P] Child has exited with status: %d\n", WEXITSTATUS(status));
    }

    printf("[%s] End of experiment\n", fc);
    
    if (check_fork(child_pid) == FORK_CHILD) {
        // Close pipe in child before exit
        close(sync_pipe[0]);
        close(sync_pipe[1]);
        exit(0);
    } else if (check_fork(child_pid) == FORK_PARENT) {
        // Close pipe in parent after experiment
        close(sync_pipe[0]);
        close(sync_pipe[1]);
    }
}

int main() {
    struct Experiment experiments[] = {
        {2, "Pipe Synchronization", 1, 3, 1, true},
        {1, "No Wait Example", 1, 3, 1, false},
    };

    size_t nof_experiments = sizeof(experiments)/sizeof(struct Experiment);

    print_process("M");

    for (size_t e = 0; e < nof_experiments; e++) {
        printf("%s\n", experiments[e].description);
        printf("------\n");
        for (size_t r = 0; r < experiments[e].repetitions; r++) {
            printf("\t----%zu\n", r);
            experiment(&experiments[e]);
            printf("\t----%zu\n", r);
        }
        printf("------\n");
    }
    
    return 0;
}