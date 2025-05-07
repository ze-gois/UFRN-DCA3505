#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stdlib.h>

extern char **environ;

//https://www.geeksforgeeks.org/wait-system-call-c/
int main() {
    pid_t pid = getpid();
    pid_t gfpid= getppid();

    printf("Pid: %d (%d)\n",pid,gfpid);

    int y = 3;

    int gsstat;

    pid_t gspid = fork();

    int x;

    if (gspid > 0){
        printf("P: Luke, I am your father!\n");
        x = 1;
        printf("waiting child:\n");
        wait(&gsstat);
        printf("--------\n");
    } else if (gspid == 0){
        printf("Gs: Noooooooooooooo!!!\n");
        x = 2;
        char *const *argv;
        char *const *envp;
        argv = (char *const[]) { "/usr/bin/ls","-la", NULL };
        envp = (char *const *) environ;
        execve("/usr/bin/ls", argv, envp);
        sleep(10);
        exit(1);
    }

    dup2(0,1);

    printf("x = %d, %d\n", x, y);

    return 0;
}
