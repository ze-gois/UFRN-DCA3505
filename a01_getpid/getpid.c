#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    pid_t pid = getpid();;
    pid_t ppid= getppid();;
    
    printf("Pid: %d (%d)\n",pid,ppid);

    int y = 3;

    pid_t gspid = fork();

    int x;

    if (gspid > 0){
        printf("Luke, I am your father!\n");
        x = 1;
    } else {
        printf("Noooooooooooooo!!!\n");
        x = 2; 
    }

    printf("x = %d, %d\n", x, y);

    while (1 == 1) {}

    return 0;
}