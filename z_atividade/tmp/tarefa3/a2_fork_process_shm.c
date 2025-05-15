#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

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

// Structure for shared memory
typedef struct {
    int ready;  // Flag to indicate synchronization status
} shared_data_t;

// Shared memory pointer
shared_data_t *shared_data = NULL;

// Initialize shared memory
bool init_lock() {
    // Create shared memory segment
    // Use shared anonymous mapping that will be inherited across fork
    shared_data = mmap(NULL, sizeof(shared_data_t), 
                      PROT_READ | PROT_WRITE, 
                      MAP_SHARED | MAP_ANONYMOUS, 
                      -1, 0);
                      
    if (shared_data == MAP_FAILED) {
        perror("mmap");
        return false;
    }
    
    // Initialize the shared data
    shared_data->ready = 0;
    
    return true;
}

// Signal readiness through shared memory
void signal_ready() {
    if (shared_data) {
        shared_data->ready = 1;
    }
}

// Wait for readiness through shared memory
void wait_ready() {
    if (shared_data) {
        while (shared_data->ready == 0) {
            // Poll the shared memory
            usleep(10000);  // 10ms sleep to reduce CPU usage
        }
    }
}

// Clean up shared memory
void cleanup_shm() {
    if (shared_data) {
        munmap(shared_data, sizeof(shared_data_t));
        shared_data = NULL;
    }
}

void experiment(struct Experiment *e) {
    // Initialize the synchronization mechanism
    if (!init_lock()) {
        fprintf(stderr, "Failed to initialize shared memory\n");
        return;
    }

    pid_t child_pid = fork();
    char fc[2];

    switch (check_fork(child_pid)) {
        case FORK_FAIL:
            perror("fork");
            cleanup_shm();
            break;

        case FORK_CHILD:
            strcpy(fc, "C");
            print_process(fc);
            sleep_process(fc, e->sleep_child);
            
            // Child notifies parent it's done with critical section
            printf("[C] Setting ready flag in shared memory\n");
            signal_ready();
            break;

        case FORK_PARENT:
            strcpy(fc, "P");
            print_process(fc);
            sleep_process(fc, e->sleep_parent);
            
            // Parent waits for child to finish critical section
            printf("[P] Waiting for child to set ready flag in shared memory\n");
            wait_ready();
            printf("[P] Ready flag set by child\n");
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
        // Child cleanup before exit
        exit(0);
    } else if (check_fork(child_pid) == FORK_PARENT) {
        // Parent cleanup
        cleanup_shm();
    }
}

int main() {
    struct Experiment experiments[] = {
        {2, "Shared Memory Sync", 1, 3, 1, true},
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