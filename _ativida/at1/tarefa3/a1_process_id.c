/*
 * a1_process_id.c - Demonstração Básica de Identificação de Processos
 * 
 * Este programa demonstra como obter o identificador do processo atual (PID)
 * e o identificador do processo pai (PPID) utilizando as funções getpid() e getppid().
 * 
 * Objetivos didáticos:
 * 1. Entender o conceito de PID e PPID
 * 2. Observar como cada execução do programa recebe um PID único
 * 3. Notar como o PPID permanece constante para múltiplas execuções do mesmo terminal
 */
#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t pid = getpid();
    pid_t ppid = getppid();

    printf("Current process ID (PID): %d\n", pid);
    printf("Parent process ID (PPID): %d\n", ppid);
    
    // Sleep to allow time to observe process
    sleep(15);

    return 0;
}