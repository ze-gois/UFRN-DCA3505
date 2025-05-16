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
    uint64_t io; // interval of observations
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

void costume(char d[]){
    char nome[32];
    snprintf(nome, 32, "t9_%s",d);
    prctl(PR_SET_NAME, (unsigned long) nome, 0, 0, 0);
}

struct Seelog {
    int file_descriptor;
    char filename[64];
    struct Experiment *seed;
} Seelog;

#define LOG_DIR "./log"

struct Seelog init_seelog(int p, struct Experiment *seed){
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

void shell(int file_descriptor, char command[], char c[]){
    pid_t f = fork();

    if (f==0){
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
                return;
            }

        execve("/usr/bin/sh", argv, environ);
        exit(0);
        perror("execve");
    }
}

void ps(struct Seelog pseer){
    // shell(pseer.file_descriptor, "ps -o pid,pri,ni,stat,%cpu,cmd --sort=-%cpu | grep -E ' ./spawn($|_[0-9]+_[0-9]+)$'","pseer");
    shell(pseer.file_descriptor, "ps -o pid,pri,ni,stat,%cpu,cmd --sort=-%cpu | grep -E 't9_.*'","pseer");
}

bool sensus(struct Seelog seelog){
    pid_t sensid = fork();
    if (sensid == 0){
        costume("sensus");
        ps(seelog);
        exit(0);
    } else if (sensid > 0) {
        return true;
    }
    return false;
}

void pseer(struct Experiment *seed){
    if (seed->no <= 0) return;

    pid_t pidseer = fork();

    if (pidseer == 0){
        costume("overseing");
        for (int p = 0; p < seed->no; p++ ){
            struct Seelog seelog = init_seelog(p,seed);
            sensus(seelog);
            usleep(seed->io);
        }
        exit(0);
    } else {
        seed->pidseer = pidseer;
    }
    return;
}

void monotono (struct Experiment *e){
    while (1);
    exit(0);
}

void parte_1(struct Experiment *e){
    e->sub = malloc(e->np*sizeof(struct Experiment *));

    for (int p = 0; p<e->np; p++){
        struct Experiment exp = { 1, '1', "Monotono", 1, monotono, 0, 0, 0, NULL, -1};
        *(e->sub+p) = &exp;
    }

    for (int p = 0; p < e->np; p++){
        pid_t subid = fork();

        if (subid == 0){
            char nome[32];
            snprintf(nome, 32, "%c_%d",e->d, p);
            costume(nome);
            (*(e->sub+p))->task(NULL);
            exit(0);
        } else {
            (*(e->sub+p))->pid = subid;
        }
    }

    wait(&e->pidseer);
    printf("-------!!!!?\n");
    fflush(stdout);

    for (int p = 0; p < e->np; p++){
        kill((*(e->sub+p))->pid, SIGKILL);
        free(e->sub+p);
    }
}

void parte_2(struct Experiment *e){
    int np = 1 + e->np;
    e->sub = malloc(np*sizeof(struct Experiment));

    for (int p = 0; p < np; p++){
        struct Experiment exp = { 1, '1', "Monotono", 1, monotono, 0, 0, 0, NULL, -1};
        *(e->sub+p) = &exp;
    }

    for (int p = 0; p < np; p++){
        pid_t subid = fork();
        (*(e->sub+p))->pid = subid;
        if (subid == 0){
            (*(e->sub+p))->task(NULL);
            exit(0);
        } else {
            (*(e->sub+p))->pid = subid;
        }
    }

    wait(&e->pidseer);

    for (int p = 0; p < np; p++){
        kill((*(e->sub+p))->pid, SIGKILL);
        free(e->sub+p);
    }
}

void parte_3(struct Experiment *e){
    int np = 1 + e->np;
    e->sub = malloc(np*sizeof(struct Experiment));

    for (int p = 0; p < np; p++){
        struct Experiment exp = { 1, '1', "Monotono", 1, monotono, 0, 0, 0, NULL, -1};
        *(e->sub+p) = &exp;
    }

    for (int p = 0; p < np; p++){
        pid_t subid = fork();
        if (subid == 0){
            char nome[32];
            snprintf(nome, 32, "%c_%d",e->d, p);
            costume(nome);
            (*(e->sub+p))->task(NULL);
            exit(0);
        } else {
            (*(e->sub+p))->pid = subid;
        }
    }

    pid_t extra = fork();
    if (extra == 0){
        char cmd[32];
        snprintf(cmd, 32, "renice -n -10 -p %d",(*(e->sub+5))->pid);
        shell(-1,cmd,"renice");
    }

    wait(&e->pidseer);

    for (int p = 0; p < np; p++){
        kill((*(e->sub+p))->pid, SIGKILL);
        free(e->sub+p);
    }
}

void parte_4(struct Experiment *e){
    e->sub = malloc(e->np*sizeof(struct Experiment));

    for (int p = 0; p<e->np; p++){
        struct Experiment exp = { 1, '1', "Monotono", 1, monotono, 0, 0, 0, NULL, -1};
        *(e->sub+p) = &exp;
    }

    for (int p = 0; p < e->np; p++){
        pid_t subid = fork();
        if (subid == 0){
            char nome[32];
            snprintf(nome, 32, "%c_%d",e->d, p);
            costume(nome);
            (*(e->sub+p))->task(NULL);
            exit(0);
        } else {
            (*(e->sub+p))->pid = subid;
        }
    }

    pid_t extra = fork();
    if (extra == 0){
        shell(-1, "./human_entry.sh","entrada");
    }

    wait(&e->pidseer);

    for (int p = 0; p < e->np; p++){
        kill((*(e->sub+p))->pid, SIGKILL);
        free(e->sub+p);
    }
}

int main() {
    int np = nproc();

    struct Experiment overseer = { 1, 'O', "Overseer", 1, NULL, 0, 0, getpid(), NULL, -1};

    struct Experiment parte1 = { 1, '1', "Parte 1", np, parte_1, 10, 5, 1000, NULL, -1};
    struct Experiment parte2 = { 2, '2', "Parte 2", np, parte_2, 10, 5, 1000, NULL, -1};
    struct Experiment parte3 = { 3, '3', "Parte 3", np, parte_3, 10, 5, 1000, NULL, -1};
    struct Experiment parte4 = { 4, '4', "Parte 4", np, parte_4, 10, 5, 1000, NULL, -1};

    struct Experiment *partes[] = { &parte1, &parte2, &parte3, &parte4 };

    overseer.sub = partes;

    int npartes = 4;

    for (int r = 0; r < overseer.r; r++){
        for (int o = 0; o < npartes; o++){
            struct Experiment *currex = *(overseer.sub+o);
            pseer(currex);
            currex->task(currex);
        }
    }

    return 0;
}
