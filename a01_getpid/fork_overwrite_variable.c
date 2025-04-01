#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    pid_t ppid= getppid();;
    printf("Pai Pid (shell): %d\n",ppid);
    
    pid_t pid = getpid();;
    printf("Filho Pid (atual): %d\n",pid);

    int y = 3;

    pid_t gspid = fork();

    int x;

    if (gspid > 0){
        printf("Filho: Luke, I am your father!\n");
        x = 1;
        y += 10;
    } else {
        printf("Neto: Noooooooooooooo!!!\n");
        x = 2; 
        y += 100;
    }

    printf("x = %d, %d\n", x, y);

    pid_t ggspid = fork();

    int x2;

    if (gspid > 0){
        if (ggspid > 0){
            printf("Luke, I am your father!\n");
            x = 71;
            y += 10;
        } else {
            printf("Noooooooooooooo!!!\n");
            x = 72; 
            y += 100;
        }
    } else {
        if (ggspid > 0){
            printf("Luke, I am your father!\n");
            x = 71;
            y += 10;
        } else {
            printf("Noooooooooooooo!!!\n");
            x = 72; 
            y += 100;
        }
    }

    while (1 == 1) {}

    return 0;
}