#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FIFO_PATH "/tmp/sync_fifo"

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

// Initialize FIFO synchronization mechanism
bool init_lock() {
    // Create named pipe (FIFO)
    if (mkfifo(FIFO_PATH, 0666) == -1) {
        // If FIFO already exists, that's okay
        if (errno != EEXIST) {
            perror("mkfifo");
            return false;
        }
    }
    return true;
}

// Signal through the FIFO
bool signal_fifo() {
    int fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("open");
        return false;
    }
    
    char buf = 'X';
    if (write(fd, &buf, 1) != 1) {
        perror("write");
        close(fd);
        return false;
    }
    
    close(fd);
    return true;
}

// Wait for signal through the FIFO
bool wait_fifo() {
    int fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return false;
    }
    
    char buf;
    if (read(fd, &buf, 1) != 1) {
        perror("read");
        close(fd);
        return false;
    }
    
    close(fd);
    return true;
}

void experiment(struct Experiment *e) {
    // Initialize the synchronization mechanism
    if (!init_lock()) {
        fprintf(stderr, "Failed to initialize FIFO\n");
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
            printf("[C] Signaling through FIFO\n");
            signal_fifo();
            break;

        case FORK_PARENT:
            strcpy(fc, "P");
            print_process(fc);
            sleep_process(fc, e->sleep_parent);
            
            // Parent waits for child to finish critical section
            printf("[P] Waiting for child signal through FIFO\n");
            wait_fifo();
            printf("[P] Signal received from child through FIFO\n");
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
        exit(0);
    } else if (check_fork(child_pid) == FORK_PARENT) {
        // Clean up FIFO after the last experiment
        // Note: In real code, you might want more robust cleanup
    }
}

int main() {
    struct Experiment experiments[] = {
        {2, "FIFO Synchronization", 1, 3, 1, true},
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
    
    // Clean up FIFO at the end
    unlink(FIFO_PATH);
    
    return 0;
}