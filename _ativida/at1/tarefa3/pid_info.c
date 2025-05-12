#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t pid = getpid();
    pid_t ppid = getppid();
    
    printf("Informações do Processo:\n");
    printf("PID (Process ID): %d\n", pid);
    printf("PPID (Parent Process ID): %d\n", ppid);
    
    return 0;
}