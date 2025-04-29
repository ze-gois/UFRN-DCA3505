// GNU C standard library includes
#include <stdio.h>      // io (i.e: printf)
#include <sys/types.h>  // types (i.e: pid_t)
#include <unistd.h>     // sycalls (i.e: fork)
#include <sys/wait.h>   // wait (i.e: wait)

enum FORK_RETURN {
    FORK_NO_PROCESS = 0,
    FORK_PARENT = 1,
    FORK_CHILD = 2,
};

enum SEQUENCE {
    SEQUENCE_ZERO = 0,
    SEQUENCE_FIRST = 1,
    SEQUENCE_SECOND = 2,
};

enum FORK_RETURN get_fork_result(pid_t fork_result) {
    if (fork_result < 0) {
        return FORK_NO_PROCESS;
    } else if (fork_result > 0) {
        return FORK_PARENT;
    } else {
        return FORK_CHILD;
    }
}

int main() {
    pid_t process_identifier = getpid();

    enum SEQUENCE sequence = SEQUENCE_ZERO;

    pid_t process_child_identifier = fork();

    switch (get_fork_result(process_child_identifier)) {
        case FORK_NO_PROCESS:
            sequence = SEQUENCE_ZERO;
            return -1;

        case FORK_PARENT:
            sequence = SEQUENCE_FIRST;
            break;

        case FORK_CHILD:
            sequence = SEQUENCE_SECOND;
            break;
    }

    switch (sequence) {
        case SEQUENCE_FIRST:
            wait(&process_child_identifier);
            printf("1");
            fflush(NULL);
            break;

        case SEQUENCE_SECOND:
            printf("2");
            fflush(NULL);
            break;
        default:
            break;
    }
    return 0;
}
