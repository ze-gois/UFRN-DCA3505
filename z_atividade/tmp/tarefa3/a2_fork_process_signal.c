#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

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

// Global flag to indicate signal receipt
volatile sig_atomic_t signal_received = 0;

// Signal handler function
void signal_handler(int signum) {
    if (signum == SIGUSR1) {
        signal_received = 1;
    }
}

// Initialize signal handling
bool init_lock() {
    struct sigaction sa;
    
    // Set up the signal handler
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    
    // Register the signal handler
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return false;
    }
    
    // Reset the signal received flag
    signal_received = 0;
    
    return true;
}

// Send a signal to a process
bool send_signal(pid_t pid) {
    if (kill(pid, SIGUSR1) == -1) {
        perror("kill");
        return false;
    }
    return true;
}

// Wait for signal
bool wait_for_signal() {
    // Wait until the signal is received
    while (!signal_received) {
        // Sleep a bit to avoid busy-waiting
        usleep(10000);  // 10ms
    }
    return true;
}

void experiment(struct Experiment *e) {
    // Initialize the signal handling
    if (!init_lock()) {
        fprintf(stderr, "Failed to initialize signal handling\n");
        return;
    }

    pid_t child_pid = fork();
    char fc[2];
    pid_t parent_pid = getpid();  // Store parent's PID before fork

    switch (check_fork(child_pid)) {
        case FORK_FAIL:
            perror("fork");
            break;

        case FORK_CHILD:
            strcpy(fc, "C");
            print_process(fc);
            sleep_process(fc, e->sleep_child);
            
            // Child signals parent it's done with critical section
            printf("[C] Sending SIGUSR1 to parent (PID: %d)\n", parent_pid);
            send_signal(parent_pid);
            break;

        case FORK_PARENT:
            strcpy(fc, "P");
            print_process(fc);
            sleep_process(fc, e->sleep_parent);
            
            // Parent waits for child to finish critical section
            printf("[P] Waiting for SIGUSR1 from child\n");
            wait_for_signal();
            printf("[P] Received SIGUSR1 from child\n");
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
    }
}

int main() {
    struct Experiment experiments[] = {
        {2, "Signal Synchronization", 1, 3, 1, true},
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