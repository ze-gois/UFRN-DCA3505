#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>

#define LOCK_FILE "/tmp/fork_example_lock"

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

// File descriptor for lock file
int lock_fd = -1;

// Initialize file lock
bool init_lock() {
    // Create or open the lock file
    lock_fd = open(LOCK_FILE, O_RDWR | O_CREAT, 0666);
    if (lock_fd == -1) {
        perror("open");
        return false;
    }
    
    // Write some content to the file
    if (write(lock_fd, "lock", 4) == -1) {
        perror("write");
        close(lock_fd);
        return false;
    }
    
    return true;
}

// Acquire lock using flock
bool acquire_lock() {
    printf("Attempting to acquire lock...\n");
    if (flock(lock_fd, LOCK_EX) == -1) {
        perror("flock (acquire)");
        return false;
    }
    printf("Lock acquired\n");
    return true;
}

// Release lock using flock
bool release_lock() {
    printf("Releasing lock...\n");
    if (flock(lock_fd, LOCK_UN) == -1) {
        perror("flock (release)");
        return false;
    }
    printf("Lock released\n");
    return true;
}

// Alternative: Use fcntl for locking
bool fcntl_lock(bool exclusive) {
    struct flock fl;
    
    fl.l_type = exclusive ? F_WRLCK : F_RDLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;  // Lock entire file
    
    printf("Attempting to acquire %s lock with fcntl...\n", 
           exclusive ? "exclusive" : "shared");
    
    if (fcntl(lock_fd, F_SETLKW, &fl) == -1) {
        perror("fcntl (lock)");
        return false;
    }
    
    printf("fcntl lock acquired\n");
    return true;
}

// Release fcntl lock
bool fcntl_unlock() {
    struct flock fl;
    
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;  // Unlock entire file
    
    printf("Releasing fcntl lock...\n");
    
    if (fcntl(lock_fd, F_SETLK, &fl) == -1) {
        perror("fcntl (unlock)");
        return false;
    }
    
    printf("fcntl lock released\n");
    return true;
}

// Cleanup resources
void cleanup() {
    if (lock_fd != -1) {
        close(lock_fd);
        lock_fd = -1;
    }
    unlink(LOCK_FILE);
}

void experiment(struct Experiment *e) {
    // Initialize the lock file
    if (!init_lock()) {
        fprintf(stderr, "Failed to initialize lock file\n");
        return;
    }

    pid_t child_pid = fork();
    char fc[2];

    switch (check_fork(child_pid)) {
        case FORK_FAIL:
            perror("fork");
            cleanup();
            break;

        case FORK_CHILD:
            strcpy(fc, "C");
            print_process(fc);
            
            // Child acquires lock, does work, then releases
            printf("[C] Waiting to acquire lock\n");
            acquire_lock();
            
            sleep_process(fc, e->sleep_child);
            printf("[C] Done with critical section\n");
            
            release_lock();
            break;

        case FORK_PARENT:
            strcpy(fc, "P");
            print_process(fc);
            sleep_process(fc, e->sleep_parent);
            
            // Parent also tries to acquire the same lock
            // This will block until child releases it
            printf("[P] Waiting to acquire lock\n");
            acquire_lock();
            printf("[P] Acquired lock after child released it\n");
            
            // Do some work with the lock
            sleep(1);
            
            // Release the lock
            release_lock();
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
        cleanup();
    }
}

// Alternative experiment using fcntl locks
void experiment_fcntl(struct Experiment *e) {
    // Initialize the lock file
    if (!init_lock()) {
        fprintf(stderr, "Failed to initialize lock file\n");
        return;
    }

    pid_t child_pid = fork();
    char fc[2];

    switch (check_fork(child_pid)) {
        case FORK_FAIL:
            perror("fork");
            cleanup();
            break;

        case FORK_CHILD:
            strcpy(fc, "C");
            print_process(fc);
            
            // Child acquires lock, does work, then releases
            printf("[C] Waiting to acquire fcntl lock\n");
            fcntl_lock(true);  // exclusive lock
            
            sleep_process(fc, e->sleep_child);
            printf("[C] Done with critical section\n");
            
            fcntl_unlock();
            break;

        case FORK_PARENT:
            strcpy(fc, "P");
            print_process(fc);
            sleep_process(fc, e->sleep_parent);
            
            // Parent also tries to acquire the same lock
            printf("[P] Waiting to acquire fcntl lock\n");
            fcntl_lock(true);  // exclusive lock
            printf("[P] Acquired fcntl lock after child released it\n");
            
            // Do some work with the lock
            sleep(1);
            
            // Release the lock
            fcntl_unlock();
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
        cleanup();
    }
}

int main() {
    struct Experiment experiments[] = {
        {1, "flock Synchronization", 1, 3, 1, true},
        {1, "fcntl Synchronization", 1, 3, 1, true},
    };

    size_t nof_experiments = sizeof(experiments)/sizeof(struct Experiment);

    print_process("M");

    printf("%s\n", experiments[0].description);
    printf("------\n");
    for (size_t r = 0; r < experiments[0].repetitions; r++) {
        printf("\t----%zu\n", r);
        experiment(&experiments[0]);
        printf("\t----%zu\n", r);
    }
    printf("------\n");
    
    printf("%s\n", experiments[1].description);
    printf("------\n");
    for (size_t r = 0; r < experiments[1].repetitions; r++) {
        printf("\t----%zu\n", r);
        experiment_fcntl(&experiments[1]);
        printf("\t----%zu\n", r);
    }
    printf("------\n");
    
    return 0;
}