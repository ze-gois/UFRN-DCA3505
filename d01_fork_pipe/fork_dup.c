#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

extern char **environ;

//https://www.geeksforgeeks.org/wait-system-call-c/
int main() {
    pid_t pid = getpid();
    pid_t gfpid= getppid();

    printf("Pid: %d (%d)\n",pid,gfpid);

    int y = 3;

    int gsstat;

    int fd = open("saida.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

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

    printf("x = %d, %d\n", x, y);

    return 0;
}
