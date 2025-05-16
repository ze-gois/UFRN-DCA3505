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
#include <errno.h>

extern char **environ;
extern char *program_invocation_short_name;

struct Experiment {
    size_t r; // repetitions
    char d; // description
    char desc[64];
    uint8_t np; // number of processes
    void (*task)(struct Experiment *);
    uint8_t no; // number of observations
    uint64_t io; // interval of observations
    pid_t pid; // experiment pid
    pid_t *child_pids; // array to store child process PIDs
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

void costume(char d[]) {
    char nome[32];
    snprintf(nome, 32, "t9_%s", d);

    // Set process name using prctl
    if (prctl(PR_SET_NAME, (unsigned long) nome, 0, 0, 0) == -1) {
        perror("prctl failed");
    }

    // We don't need to modify argv[0] as process comm name is enough

    // Print confirmation of name change
    char current_name[32];
    if (prctl(PR_GET_NAME, (unsigned long) current_name, 0, 0, 0) != -1) {
        printf("Process name set to: %s (PID: %d)\n", current_name, getpid());
    }
}

struct Seelog {
    int file_descriptor;
    char filename[64];
    struct Experiment *seed;
} Seelog;

#define LOG_DIR "./log"

struct Seelog init_seelog(int p, struct Experiment *seed) {
    struct Seelog seelog = { -1, "", seed };

    mkdir(LOG_DIR, 0755);

    time_t timestamp;
    time(&timestamp);
    struct tm *tm_info = localtime(&timestamp);

    char base_filename[64];
    snprintf(base_filename, sizeof(base_filename),
             "%s/%c-%d",
             LOG_DIR, seed->d, p);

    char timestamp_str[64];
    strftime(timestamp_str, sizeof(timestamp_str),
             "-%Y-%m-%d-%H-%M-%S.log", tm_info);

    snprintf(seelog.filename, sizeof(seelog.filename),
             "%s%s", base_filename, timestamp_str);

    seelog.file_descriptor = open(seelog.filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (seelog.file_descriptor == -1) {
        perror("Failed to create log file");
    }

    return seelog;
}

void shell(int file_descriptor, char command[], char c[]) {
    pid_t f = fork();

    if (f == 0) {
        costume(c);
        char *argv[] = {
            "/usr/bin/sh",
            "-c",
            command,
            NULL
        };

        if (file_descriptor != STDOUT_FILENO)
            if (dup2(file_descriptor, STDOUT_FILENO) == -1) {
                perror("dup2");
                exit(1);
            }

        execve("/usr/bin/sh", argv, environ);
        perror("execve");
        exit(1);
    } else if (f > 0) {
        // Wait for the shell command to complete
        int status;
        waitpid(f, &status, 0);
    } else {
        perror("fork failed");
    }
}

void ps(struct Seelog pseer) {
    // First show header
    shell(pseer.file_descriptor, "ps -e -o pid,pri,ni,stat,%cpu,comm,cmd | head -n 1", "pseer_header");
    // Show only our t9_* processes
    shell(pseer.file_descriptor, "ps -e -o pid,pri,ni,stat,%cpu,comm,cmd | grep 't9_'", "pseer_filtered");
    // Then show top t9_* processes by CPU usage
    shell(pseer.file_descriptor, "echo '\n--- TOP t9_* CPU CONSUMERS ---'", "pseer_separator");
    shell(pseer.file_descriptor, "ps -e -o pid,pri,ni,stat,%cpu,comm,cmd --sort=-%cpu | grep 't9_' | head -n 10", "pseer_top");
}

void sensus(struct Seelog seelog) {
    pid_t sensid = fork();
    if (sensid == 0) {
        costume("sensus");
        // Give time for the main processes to start and establish themselves
        usleep(500000);  // 500ms delay
        ps(seelog);
        exit(0);
    } else if (sensid > 0) {
        // Wait for the sensus process to complete
        int status;
        waitpid(sensid, &status, 0);
        return;
    }
    return;
}

void pseer(struct Experiment *seed) {
    if (seed->no <= 0) return;

    pid_t pidseer = fork();

    if (pidseer == 0) {
        costume("overseing");
        for (int p = 0; p < seed->no; p++) {
            struct Seelog seelog = init_seelog(p, seed);
            sensus(seelog);
            if (seelog.file_descriptor != -1) {
                close(seelog.file_descriptor);
            }
            usleep(seed->io * 1000); // Convert to microseconds
        }
        exit(0);
    } else {
        seed->pidseer = pidseer;
    }
    return;
}

// Function for CPU-intensive processes
void monotono(struct Experiment *e) {
    // Set process name again, just to be sure
    char nome[32];
    if (e != NULL && e->d != 0) {
        snprintf(nome, 32, "t9_monotono_%c", e->d);
    } else {
        strcpy(nome, "t9_monotono");
    }

    // Set process name using prctl
    if (prctl(PR_SET_NAME, (unsigned long) nome, 0, 0, 0) == -1) {
        perror("prctl failed");
    }

    // Confirm name was set properly
    char current_name[32];
    if (prctl(PR_GET_NAME, (unsigned long) current_name, 0, 0, 0) != -1) {
        printf("CPU process running as: %s (PID: %d)\n", current_name, getpid());
    }

    // Infinite CPU-intensive loop
    while (1) {
        // More intensive CPU work to ensure high CPU usage
        volatile double result = 0.0;
        for (volatile int i = 0; i < 10000000; i++) {
            result += i * i / (i + 1.0);
        }
    }
    exit(0); // Never reached
}

void parte_1(struct Experiment *e) {
    // Allocate array to store child PIDs
    e->child_pids = malloc(e->np * sizeof(pid_t));
    if (e->child_pids == NULL) {
        perror("Failed to allocate memory for child PIDs");
        return;
    }

    // Launch N CPU-intensive processes
    for (int p = 0; p < e->np; p++) {
        pid_t subid = fork();

        if (subid == 0) {
            char nome[32];
            snprintf(nome, 32, "%c_%d", e->d, p);
            costume(nome);
            monotono(NULL); // This will never return
            exit(0);
        } else {
            e->child_pids[p] = subid;
            printf("Started process %d with PID %d\n", p, subid);
        }
    }

    // Wait for the observer process to finish
    int status;
    waitpid(e->pidseer, &status, 0);

    printf("Observer process exited, cleaning up experiment processes\n");
    fflush(stdout);

    // Give some time for the ps command to finish observing
    sleep(1);

    // Clean up child processes
    for (int p = 0; p < e->np; p++) {
        printf("Killing process with PID %d\n", e->child_pids[p]);
        kill(e->child_pids[p], SIGKILL);
    }

    // Free the memory allocated for child PIDs
    free(e->child_pids);
}

void parte_2(struct Experiment *e) {
    int np = 1 + e->np; // N+1 processes

    // Allocate array to store child PIDs
    e->child_pids = malloc(np * sizeof(pid_t));
    if (e->child_pids == NULL) {
        perror("Failed to allocate memory for child PIDs");
        return;
    }

    // Launch N+1 CPU-intensive processes
    for (int p = 0; p < np; p++) {
        pid_t subid = fork();
        if (subid == 0) {
            char nome[32];
            snprintf(nome, 32, "%c_%d", e->d, p);
            costume(nome);

            // Create an experiment struct for this process
            struct Experiment this_proc = { 0, e->d, "Monotono", 0, NULL, 0, 0, getpid(), NULL, -1 };
            monotono(&this_proc); // This will never return
            exit(0);
        } else {
            e->child_pids[p] = subid;
            printf("Started process %d with PID %d\n", p, subid);
        }
    }

    // Wait for the observer process to finish
    int status;
    waitpid(e->pidseer, &status, 0);

    printf("Observer process exited, cleaning up experiment processes\n");
    fflush(stdout);

    // Clean up child processes
    for (int p = 0; p < np; p++) {
        printf("Killing process with PID %d\n", e->child_pids[p]);
        kill(e->child_pids[p], SIGKILL);
    }

    // Free the memory allocated for child PIDs
    free(e->child_pids);
}

void parte_3(struct Experiment *e) {
    int np = 1 + e->np; // N+1 processes

    // Allocate array to store child PIDs
    e->child_pids = malloc(np * sizeof(pid_t));
    if (e->child_pids == NULL) {
        perror("Failed to allocate memory for child PIDs");
        return;
    }

    // Launch N+1 CPU-intensive processes
    for (int p = 0; p < np; p++) {
        pid_t subid = fork();
        if (subid == 0) {
            char nome[32];
            snprintf(nome, 32, "%c_%d", e->d, p);
            costume(nome);
            monotono(NULL); // This will never return
            exit(0);
        } else {
            e->child_pids[p] = subid;
            printf("Started process %d with PID %d\n", p, subid);
        }
    }

    // Increase priority of process 5 (or the last one if np < 5)
    int target_proc = (5 < np) ? 5 : np - 1;
    pid_t renice_pid = fork();
    if (renice_pid == 0) {
        char cmd[64];
        snprintf(cmd, sizeof(cmd), "renice -n -10 -p %d", e->child_pids[target_proc]);
        printf("Running: %s\n", cmd);
        shell(-1, cmd, "renice");
        exit(0);
    } else {
        // Wait for the renice process to complete
        int status;
        waitpid(renice_pid, &status, 0);
    }

    // Wait for the observer process to finish
    waitpid(e->pidseer, NULL, 0);

    printf("Observer process exited, cleaning up experiment processes\n");
    fflush(stdout);

    // Clean up child processes
    for (int p = 0; p < np; p++) {
        printf("Killing process with PID %d\n", e->child_pids[p]);
        kill(e->child_pids[p], SIGKILL);
    }

    // Free the memory allocated for child PIDs
    free(e->child_pids);
}

void parte_4(struct Experiment *e) {
    // Allocate array to store child PIDs
    e->child_pids = malloc((e->np + 1) * sizeof(pid_t)); // +1 for the blocking process
    if (e->child_pids == NULL) {
        perror("Failed to allocate memory for child PIDs");
        return;
    }

    // Launch N CPU-intensive processes
    for (int p = 0; p < e->np; p++) {
        pid_t subid = fork();
        if (subid == 0) {
            char nome[32];
            snprintf(nome, 32, "%c_%d", e->d, p);
            costume(nome);
            monotono(NULL); // This will never return
            exit(0);
        } else {
            e->child_pids[p] = subid;
            printf("Started CPU-intensive process %d with PID %d\n", p, subid);
        }
    }

    // Create a named pipe for communication with the blocking process
    char fifo_path[64];
    snprintf(fifo_path, sizeof(fifo_path), "/tmp/blocking_fifo_%d", getpid());
    unlink(fifo_path);  // Remove if exists
    if (mkfifo(fifo_path, 0666) == -1) {
        perror("mkfifo");
    }

    // Make sure the directory structure exists
    mkdir(LOG_DIR, 0755);

    // Launch blocking process
    pid_t blocking_pid = fork();
    if (blocking_pid == 0) {
        costume("blocking_io");

        // Redirect stdin to our named pipe
        int fd = open(fifo_path, O_RDONLY);
        if (fd != -1) {
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // Use exec with a bash command to make sure our process name appears correctly in ps
        char *argv[] = { "/bin/bash", "-c", "exec -a t9_blocking_io ./blocking.sh", NULL };
        execve("/bin/bash", argv, environ);
        perror("execve blocking.sh");
        exit(1);
    } else {
        e->child_pids[e->np] = blocking_pid;
        printf("Started blocking process with PID %d\n", blocking_pid);
    }

    // Wait for the observer process to finish
    waitpid(e->pidseer, NULL, 0);

    printf("Observer process exited, cleaning up experiment processes\n");
    fflush(stdout);

    // Give some time for the ps command to finish observing
    sleep(1);

    // Clean up child processes (N CPU-intensive + 1 blocking)
    for (int p = 0; p <= e->np; p++) {
        printf("Killing process with PID %d\n", e->child_pids[p]);
        kill(e->child_pids[p], SIGKILL);
    }

    // Free the memory allocated for child PIDs
    free(e->child_pids);
}

int main() {
    int np = nproc();
    printf("Detected %d processor cores\n", np);

    struct Experiment parte1 = { 1, '1', "Parte 1", np, parte_1, 10, 500, 0, NULL, -1 };
    struct Experiment parte2 = { 2, '2', "Parte 2", np, parte_2, 10, 500, 0, NULL, -1 };
    struct Experiment parte3 = { 3, '3', "Parte 3", np, parte_3, 10, 500, 0, NULL, -1 };
    struct Experiment parte4 = { 4, '4', "Parte 4", np, parte_4, 10, 500, 0, NULL, -1 };

    struct Experiment *partes[] = { &parte1, &parte2, &parte3, &parte4 };
    int npartes = sizeof(partes) / sizeof(partes[0]);

    for (int o = 0; o < npartes; o++) {
        struct Experiment *currex = partes[o];
        pseer(currex);
        currex->task(currex);
        sleep(2);
    }

    printf("All experiments completed\n");
    return 0;
}
