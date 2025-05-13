#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

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

// Define the union for semaphore operations (required for some systems)
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
// The union is already defined in <sys/sem.h>
#else
union semun {
    int val;                  // Value for SETVAL
    struct semid_ds *buf;     // Buffer for IPC_STAT, IPC_SET
    unsigned short *array;    // Array for GETALL, SETALL
    struct seminfo *__buf;    // Buffer for IPC_INFO (Linux-specific)
};
#endif

// Semaphore ID
int sem_id = -1;

// Initialize semaphore
bool init_lock() {
    key_t key;
    
    // Generate a key using the current directory and a predefined project ID
    if ((key = ftok(".", 'B')) == -1) {
        perror("ftok");
        return false;
    }
    
    // Create or get the semaphore set with one semaphore
    if ((sem_id = semget(key, 1, 0666 | IPC_CREAT)) == -1) {
        perror("semget");
        return false;
    }
    
    // Initialize the semaphore with value 0 (locked)
    union semun arg;
    arg.val = 0;
    
    if (semctl(sem_id, 0, SETVAL, arg) == -1) {
        perror("semctl");
        return false;
    }
    
    return true;
}

// Wait on the semaphore (P operation)
bool sem_wait() {
    struct sembuf sb = {
        .sem_num = 0,        // Semaphore number in the set (we only have one)
        .sem_op = -1,        // Decrement by 1 (wait operation)
        .sem_flg = 0,        // Block until operation can be performed
    };
    
    if (semop(sem_id, &sb, 1) == -1) {
        perror("semop (wait)");
        return false;
    }
    
    return true;
}

// Signal the semaphore (V operation)
bool sem_signal() {
    struct sembuf sb = {
        .sem_num = 0,        // Semaphore number in the set
        .sem_op = 1,         // Increment by 1 (signal operation)
        .sem_flg = 0,        // Operation flags
    };
    
    if (semop(sem_id, &sb, 1) == -1) {
        perror("semop (signal)");
        return false;
    }
    
    return true;
}

// Clean up semaphore
void cleanup_sem() {
    if (sem_id != -1) {
        // Only the parent should remove the semaphore at the end
        if (semctl(sem_id, 0, IPC_RMID, 0) == -1) {
            perror("semctl (IPC_RMID)");
        }
        sem_id = -1;
    }
}

void experiment(struct Experiment *e) {
    // Initialize the synchronization mechanism
    if (!init_lock()) {
        fprintf(stderr, "Failed to initialize semaphore\n");
        return;
    }

    pid_t child_pid = fork();
    char fc[2];

    switch (check_fork(child_pid)) {
        case FORK_FAIL:
            perror("fork");
            cleanup_sem();
            break;

        case FORK_CHILD:
            strcpy(fc, "C");
            print_process(fc);
            sleep_process(fc, e->sleep_child);
            
            // Child signals parent it's done with critical section
            printf("[C] Signaling semaphore\n");
            sem_signal();
            break;

        case FORK_PARENT:
            strcpy(fc, "P");
            print_process(fc);
            sleep_process(fc, e->sleep_parent);
            
            // Parent waits for child to finish critical section
            printf("[P] Waiting on semaphore\n");
            sem_wait();
            printf("[P] Semaphore signaled by child\n");
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
        // Child exits without cleaning up the semaphore
        exit(0);
    } else if (check_fork(child_pid) == FORK_PARENT && !e->do_parent_wait_child) {
        // Only clean up if we're not waiting for the child
        // (otherwise we'll clean up after wait())
    }
}

int main() {
    struct Experiment experiments[] = {
        {2, "Semaphore Synchronization", 1, 3, 1, true},
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
    
    // Clean up semaphore at the end
    cleanup_sem();
    
    return 0;
}