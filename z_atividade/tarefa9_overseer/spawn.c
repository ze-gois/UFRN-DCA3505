#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

struct Task;
struct Experiment;

struct Experiment {
    pid_t pid;
    char acronym;
    char description[64];
    struct Task *t;
    struct Experiment *sub[];
};

struct Task {
    uint16_t repetitions;
    uint64_t intervals;
    void (*task) (struct Experiment *);
};

void monotone() {
    while(1);
}

void see (struct Experiment *e){

}

int main(){
    printf(".\n");
    printf("1\n");
    printf("2\n");
    // struct Experiment overseer = {getpid(), 'O', "Overseer", {1, 500000, see}, NULL};

    return 0;
}
