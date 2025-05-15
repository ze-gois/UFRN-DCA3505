#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    
    // Fork and create a child process
    pid = fork();
    
    // Check if fork() was successful
    if (pid < 0) {
        // Fork failed
        fprintf(stderr, "Erro ao criar processo filho\n");
        return 1;
    } 
    else if (pid == 0) {
        // This code runs in the child process (pid = 0 in the child)
        printf("=== PROCESSO FILHO ===\n");
        printf("PID do filho: %d\n", getpid());
        printf("PPID do filho (PID do pai): %d\n", getppid());
        printf("O valor de pid para o filho é: %d\n", pid);
        
        // Adding a small delay to allow the parent process to be observed
        sleep(5);
        printf("Processo filho finalizando...\n");
    } 
    else {
        // This code runs in the parent process (pid = PID of the child)
        printf("=== PROCESSO PAI ===\n");
        printf("PID do pai: %d\n", getpid());
        printf("PPID do pai: %d\n", getppid());
        printf("PID do filho criado: %d\n", pid);
        
        // Wait for the child to finish
        int status;
        waitpid(pid, &status, 0);
        printf("Processo pai finalizando após o término do filho...\n");
    }
    
    return 0;
}