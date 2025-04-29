// GNU C standard library includes
#include <stdio.h>      // io (i.e: printf)
#include <sys/types.h>  // types (i.e: pid_t)
#include <unistd.h>     // sycalls (i.e: fork)

enum FORK_RETURN {
    NO_PROCESS = 0,
    PARENT = 1,
    CHILD = 2,
};

enum FORK_RETURN get_fork_result(pid_t fork_result) {
    if (fork_result < 0) {
        return NO_PROCESS;
    } else if (fork_result > 0) {
        return PARENT;
    } else {
        return CHILD;
    }
}

int main() {
    pid_t process_identifier = getpid();
    pid_t process_parent_identifier= getppid();

    printf("Code process identifier: %d\n", process_identifier);
    printf("Code parent identifier: %d\n", process_parent_identifier);

    int duplicated_variable = 2;

    pid_t process_child_identifier = fork();

    int individual_variable;

    switch (get_fork_result(process_child_identifier)) {
        case NO_PROCESS:
            printf("Fork syscall yield no thread\n");
            return -1;

        case PARENT:
            printf("\t\tProcess %d, I am your child.\n", process_parent_identifier);
            printf("\t\tProcess %d, I am your parent.\n", process_child_identifier);
            printf("\t\tProcess duplicated variable %d\n", duplicated_variable);
            duplicated_variable = 5;
            individual_variable = 6;
            printf("\t\tProcess individual variable %d\n", individual_variable);
            break;

        case CHILD:
            printf("\tProcess %d, I am your child.\n", process_identifier);
            printf("\tProcess %d, I am your grandchild.\n", process_parent_identifier);
            printf("\tChild's duplicated variable %d\n", duplicated_variable);
            duplicated_variable = 7;
            individual_variable = 1;
            printf("\tProcess individual variable %d\n", individual_variable);
            break;
    }

    printf("Duplicated variable %d\n", duplicated_variable);
    printf("Individual variable %d\n", individual_variable);

    return 0;
}
