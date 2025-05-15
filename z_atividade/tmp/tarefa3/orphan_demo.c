/*
 * orphan_demo.c - Demonstração de Processos Órfãos
 *
 * Este programa demonstra como criar um processo órfão.
 * Um processo órfão ocorre quando o processo pai termina antes do filho,
 * fazendo com que o filho seja "adotado" pelo processo init (PID 1).
 * 
 * Objetivos didáticos:
 * 1. Demonstrar como criar um processo órfão intencionalmente
 * 2. Observar como o PPID do processo filho muda para 1 (ou outro PID do process manager)
 * 3. Visualizar o momento em que ocorre a "adoção" pelo init
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t child_pid;
    
    printf("Programa de demonstração de processos órfãos\n");
    printf("Processo principal (antes do fork) - PID: %d, PPID: %d\n", getpid(), getppid());
    
    child_pid = fork();
    
    if (child_pid < 0) {
        perror("Falha ao criar processo filho");
        exit(EXIT_FAILURE);
    }
    
    if (child_pid == 0) {
        // Código do processo filho
        printf("[FILHO] Iniciado - PID: %d, PPID: %d\n", getpid(), getppid());
        printf("[FILHO] Vou dormir por 5 segundos enquanto meu pai termina...\n");
        
        // Pausa para garantir que o pai termine antes
        sleep(2);
        printf("[FILHO] Ainda executando - PID: %d, PPID: %d\n", getpid(), getppid());
        
        sleep(2);
        printf("[FILHO] Processo pai provavelmente terminou - PID: %d, PPID: %d\n", getpid(), getppid());
        
        // Se PPID=1 (ou outro PID do system process manager), o filho foi adotado
        if (getppid() == 1 || getppid() != child_pid) {
            printf("[FILHO] Sou um processo órfão! Fui adotado pelo init ou systemd (PPID=%d)\n", getppid());
        }
        
        sleep(1);
        printf("[FILHO] Finalizando após ser órfão - PID: %d, PPID: %d\n", getpid(), getppid());
        exit(EXIT_SUCCESS);
    } else {
        // Código do processo pai
        printf("[PAI] Criado filho com PID: %d\n", child_pid);
        printf("[PAI] Meu PID: %d, PPID: %d\n", getpid(), getppid());
        
        // Pai não espera o filho e termina imediatamente
        printf("[PAI] Vou terminar ANTES do meu filho, criando um órfão\n");
        printf("[PAI] Terminando agora...\n");
        
        // Pai finaliza, filho se torna órfão
        exit(EXIT_SUCCESS);
    }
    
    return 0; // Nunca chegará aqui
}