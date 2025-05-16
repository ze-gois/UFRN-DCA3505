#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/shm.h>

extern char **environ;

struct Experiment {
    size_t r; // repetitions
    char d; // description
    char desc[64];
    uint8_t np; // number of processes
    void (*task)(struct Experiment *);
    uint8_t no; // number of observations
    uint8_t io; // interval of observations
    pid_t pid; // experiment pid
    struct Experiment **sub;
    pid_t pidseer;
    int shm_id;  // Shared memory id
    void *shm_ptr; // Shared memory pointer
} Experiment;

int nproc() {
    FILE *fp;
    char result[16];
    int cores = 1;
    fp = popen("nproc", "r");
    if (fgets(result, sizeof(result), fp) != NULL)
        cores = atoi(result);
    pclose(fp);
    return cores;
}

void pseer (struct Experiment *seed){
    if (seed->no <= 0) return;

    // Create shared memory segment
    int shm_id = shmget(IPC_PRIVATE, sizeof(pid_t), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget failed");
        seed->pidseer = -1;
        return;
    }

    // Attach shared memory
    pid_t *shm_ptr = (pid_t *)shmat(shm_id, NULL, 0);
    if (shm_ptr == (pid_t *)-1) {
        perror("shmat failed");
        shmctl(shm_id, IPC_RMID, NULL);
        seed->pidseer = -1;
        return;
    }

    // Initialize shared memory
    *shm_ptr = -1;

    // Store shared memory id and ptr
    seed->shm_id = shm_id;
    seed->shm_ptr = shm_ptr;

    // Fork child process
    pid_t pidseer = fork();
    printf("--->%d\n", pidseer);

    if (pidseer == -1) {
        // Error in fork
        perror("fork");
        *shm_ptr = -1;  // Use shared memory
        shmdt(shm_ptr);
        shmctl(shm_id, IPC_RMID, NULL);
        return;
    } else if (pidseer == 0) {
        // Child process
        // Update shared memory with child's PID so parent knows child is ready
        *shm_ptr = getpid();
        sleep(1);  // Give parent time to see the update
        exit(0);
    } else {
        // Parent process
        seed->pidseer = pidseer;
    }

    return;
}

void monotono (){
    while (1);
}

void parte_1(struct Experiment *e){
    monotono();
}

void parte_2(struct Experiment *e){
    monotono();
}

void parte_3(struct Experiment *e){
    monotono();
}

void parte_4(struct Experiment *e){
    monotono();
}

int main() {
    int np = nproc();

    struct Experiment overseer = { 1, 'O', "Overseer", 1, NULL, 0, 0, getpid(), NULL, -1, -1, NULL};

    struct Experiment parte1 = { 1, '1', "Parte 1", np,   parte_1, 0, 5, 0, NULL, -1, -1, NULL};
    struct Experiment parte2 = { 2, '2', "Parte 2", np+1, parte_2, 0, 5, 0, NULL, -1, -1, NULL};
    struct Experiment parte3 = { 3, '3', "Parte 3", np+1, parte_3, 0, 5, 0, NULL, -1, -1, NULL};
    struct Experiment parte4 = { 4, '4', "Parte 4", np+1, parte_4, 0, 5, 0, NULL, -1, -1, NULL};

    struct Experiment *partes[] = { &parte1, &parte2, &parte3, &parte4 };

    overseer.sub = partes;

    int npartes = 4;

    for (int r = 0; r < overseer.r; r++){
        for (int o = 0; o < npartes; o++){
            struct Experiment *currex = *(overseer.sub+o);
            pseer(currex);
            printf("a\n");

            if (currex->pidseer != -1) {
                // Wait for child process to update shared memory
                pid_t *shm_ptr = (pid_t *)currex->shm_ptr;

                // Wait until the child process updates the shared memory
                // This is now a valid shared memory wait
                while (*shm_ptr == -1) {
                    usleep(1000);  // Sleep for 1ms to avoid CPU spinning
                }

                printfOnly run task if fork was successful
                // currex->task(currex);
            }
        }
    }
    return 0;
}
