#include <cstddef>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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
    printf("[%s]: (%d,%d)\n",d,getpid(),getppid());
}

void sleep_process(char const d[], unsigned int duration){
    printf("[%s]: (%d,%d) sleep for %d seconds\n",d,getpid(),getppid(),duration);
    sleep(duration);
}

struct Lock {
    bool dictator;
    bool tooth[2];
    size_t tooth_n;
};

struct Lock locker = { false, {false,false} };

bool init_lock() {
    return true;
}

bool lock();

bool is_locked() {
    bool l = locker.dictator;
    for (int t = 0; t < 2; t++) {
        if ((l = locker.tooth[t])) return true;
    }
    return false;
}


void experiment(struct Experiment *e) {
    pid_t child_pid = fork();

    char fc[2];

    switch (check_fork(child_pid)) {
        case FORK_FAIL:
        break;

        case FORK_CHILD:
            strcpy(fc, "C");
            print_process(fc);
            sleep_process(fc,e->sleep_child);
            locker.tooth[0] = false;
        break;

        case FORK_PARENT:
            strcpy(fc, "P");
            print_process(fc);
            sleep_process(fc,e->sleep_parent);
            locker.tooth[2] = false;
        break;
    }

    sleep_process(fc, e->sleep_post);

    print_process(fc);

    if (e->do_parent_wait_child){
        wait(&child_pid);
    }

    printf("[%s] End of experiment\n", fc);
    if (child_pid == 0) {

        exit(0);
    }
}

int main() {
    struct Experiment experiments[] = {
        {2,"A",0,0,0,true},
        // {3,"B",0,0,0,false},
    };

    size_t nof_experiments = sizeof(experiments)/sizeof(struct Experiment);

    print_process("M");

    for (size_t e = 0; e < nof_experiments; e++){
        printf("%s\n",experiments[e].description);
        printf("------\n");
        for (size_t r = 0; r < experiments[e].repetitions; r++ ){
            printf("\t----%d\n",r);
            locker.tooth = {true, true};
            experiment(&experiments[e]);
            // while(p_lock || c_lock) {printf("0");};
            printf("\t----%d\n",r);
        }
        printf("------\n");
    }
}
