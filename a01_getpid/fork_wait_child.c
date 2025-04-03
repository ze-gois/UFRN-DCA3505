#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


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
        wait(&gsstat)
    } else if (gspid == 0){
        printf("Gs: Noooooooooooooo!!!\n");
        x = 2;
        sleep(3);
        exit(1);
    }

    printf("x = %d, %d\n", x, y);

    return 0;
}