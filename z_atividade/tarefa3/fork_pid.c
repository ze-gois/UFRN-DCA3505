#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct Experiment {
    size_t repetitions;
    size_t repetition_current;
    char acronym[3];
    char description[40];
    unsigned int sleep_child;
    unsigned int sleep_grandchild;
    pid_t child;
    pid_t grand_child;
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

void print_process(struct Experiment *e, char const fc[], char id) {
    printf("%s%zu\t%s\t%d\t%d\t%c\n", e->acronym, e->repetition_current, fc,getpid(),getppid(), id);
}

void sleep_process(struct Experiment *e, char const fc[], unsigned int duration){
    printf("%s%zu\t%s\t%d\t%d\tsleep %d\n", e->acronym, e->repetition_current, fc, getpid(), getppid(), duration);
    usleep(duration);
    printf("%s%zu\t%s\t%d\t%d\twokeup\n", e->acronym, e->repetition_current, fc, getpid(), getppid());
}

void experiment(struct Experiment *e) {
    pid_t child_pid = fork();

    e->child = child_pid;
    char fc[2];

    switch (check_fork(child_pid)) {
        case FORK_FAIL:
        break;
        case FORK_CHILD:
            pid_t grand_child_pid = fork();
            e->grand_child = grand_child_pid;
            switch (check_fork(grand_child_pid)) {
                case FORK_FAIL:
                break;
                case FORK_CHILD:
                    strcpy(fc, "G");
                    print_process(e, fc,'A');
                    sleep_process(e, fc, e->sleep_grandchild);
                    print_process(e, fc,'B');

                break;
                case FORK_PARENT:
                    strcpy(fc, "C");
                    print_process(e, fc,'A');
                    sleep_process(e, fc, e->sleep_child);
                    print_process(e, fc,'B');
                break;
            }
            printf("%s%zu\t%s\t0\t0\n", e->acronym, e->repetition_current,fc);
            if (child_pid == 0) {
                exit(0);
            }
        break;

        case FORK_PARENT:
            usleep(100 + (e->sleep_grandchild > e->sleep_child ? e->sleep_grandchild : e->sleep_grandchild));
        break;
    }
}

int main() {
    struct Experiment overseer = {0,0,"M\0", "Main\0", 0, 0};

    print_process(&overseer,"M",' ');

    struct Experiment experiments[] = {
        {10, 0, "A\0", "Experiment A - Sem espera\0", 0, 0}, // sem sono, race
        {5,  0, "B\0", "Experiment B - Pai espera\0", 100, 0}, // filho dorme
        {5,  0, "C\0", "Experiment C - Neto órfãos\0",0, 100}  // neto dorme
    };

    size_t nof_experiments = sizeof(experiments)/sizeof(struct Experiment);

    for (size_t e = 0; e < nof_experiments; e++){
        // printf("------------\n");
        // printf("%s\n",experiments[e].description);
        for (size_t r = 0; r < experiments[e].repetitions; r++ ){
            experiments[e].repetition_current = r;
            // printf("------------------------------------r=%zu\n",r);
            experiment(&experiments[e]);
            wait(&experiments[e].child);
            wait(&experiments[e].grand_child);
        }
    }
    // printf("----------------------------------- END ---------------------------------\n");
}
