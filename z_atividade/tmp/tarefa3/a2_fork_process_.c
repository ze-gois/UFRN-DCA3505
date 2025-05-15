#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct Experiment {
    size_t repetitions;
    char description[40];
    unsigned int sleep_parent;
    unsigned int sleep_child;
    unsigned int sleep_post;
    bool do_parent_wait_child;
    bool do_child_wait_parent;
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

void print_process(char const d[]) {
    printf("[%s]: (pid=%d, ppid=%d)\n",d,getpid(),getppid());
}

void sleep_process(char const fc[], unsigned int duration){
    printf("[%s]: (pid=%d, ppid=%d) sleep for %d seconds\n", fc, getpid(), getppid(), duration);
    sleep(duration);
    printf("[%s]: (pid=%d, ppid=%d) wokeup\n", fc, getpid(), getppid());
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
                    print_process(fc);
                    sleep_process(fc, e->sleep_child);
                    sleep_process(fc, e->sleep_post);
                    print_process(fc);

                break;
                case FORK_PARENT:
                    strcpy(fc, "C");
                    print_process(fc);
                    sleep_process(fc, e->sleep_parent);
                    sleep_process(fc, e->sleep_post);
                    print_process(fc);

                    if (e->do_parent_wait_child){
                        wait(&grand_child_pid);
                    }
                break;
            }
            printf("[%s]: End of experiment\n", fc);
            if (child_pid == 0) {
                exit(0);
            }
        break;

        case FORK_PARENT:
            sleep(e->sleep_post + (e->sleep_child > e->sleep_parent ? e->sleep_child : e->sleep_child));
        break;
    }
}

int main() {
    struct Experiment experiments[] = {
    //   r             description         c   g  p   cw     gw
        {5, "Experiment A - Proc rápidos", 0,  0, 0, false, false}, // Sem espera
        {1, "Experiment B - Sem espera",   2,  5, 1, false, false},   // Pai executa independentemente
        {2, "Experiment B - Pai espera",   0,  3, 1, false, false},    // Pai espera pelo filho
        {3, "Experiment D - Proc órfãos",  0, 10, 0, false, false}  // Intencionalmente cria órfãos
    };

    size_t nof_experiments = sizeof(experiments)/sizeof(struct Experiment);

    print_process("M");

    for (size_t e = 0; e < nof_experiments; e++){
        printf("------------\n");
        printf("%s\n",experiments[e].description);
        printf("------------\n");
        for (size_t r = 0; r < experiments[e].repetitions; r++ ){
            printf("------------------------------------r=%d\n",r);
            experiment(&experiments[e]);
            wait(&experiments[e].child);
            wait(&experiments[e].grand_child);
        }
    }

    printf("----------------------------------- END ---------------------------------\n");
}
