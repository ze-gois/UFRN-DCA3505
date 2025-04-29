// GNU C standard library includes
#include <stdio.h>      // io (i.e: printf)
#include <sys/types.h>  // types (i.e: pid_t)
#include <unistd.h>     // sycalls (i.e: fork)

enum FORK_RETURN {
    NO_PROCESS = 0,
    PARENT = 1,
    CHILD = 2,
};

enum FORK_RETURN enum_fork_return(pid_t fork_result) {
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

    switch (enum_fork_return(process_child_identifier)) {
        case NO_PROCESS:
            printf("Fork syscall yield no thread\n");
            return -1;

        case PARENT:
            printf("Process %d, I (%d) am your child.\n", process_parent_identifier,process_identifier);
            printf("Process %d, I (%d) am your parent.\n", process_child_identifier,process_identifier);
            break;

        case CHILD:
            printf("Process %d, I (%d) am your child.\n", process_identifier,process_child_identifier);
            printf("Process %d, I (%d) am your grandchild.\n", process_parent_identifier,process_child_identifier);
            break;
    }

    return 0;
}
