/*
 * a2_fork_process.c - Demonstração de Criação de Processos com fork()
 *
 * Este programa demonstra como criar um novo processo usando a função fork().
 * Tanto o processo pai quanto o filho exibem informações sobre suas identidades.
 *
 * Objetivos didáticos:
 * 1. Entender como a função fork() cria um novo processo
 * 2. Observar como o processo filho é uma cópia do pai, mas com PID diferente
 * 3. Compreender como identificar quem é o processo pai e quem é o filho
 * 4. Notar a relação hierárquica entre os processos (o PPID do filho é o PID do pai)
 * 5. Demonstrar processos órfãos quando o pai termina antes do filho
 */
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

struct Experiment {
    char description[32];        // Descrição deste experimento
    unsigned int wait_parent;    // Segundos que o pai dorme (se não esperar pelo filho)
    unsigned int wait_child;     // Segundos que o filho dorme
    unsigned int wait_post;      // Segundos para dormir após a execução principal
    bool do_parent_wait_child;   // Se verdadeiro, pai espera o filho terminar
    unsigned int repetition;     // Número de vezes para repetir este experimento
};

// Executa um experimento e garante que ambos os processos pai e filho sejam concluídos antes de retornar
void run_experiment(const struct Experiment *exp, unsigned int rep_num) {
    pid_t child_pid;
    int status;
    
    printf("\n===== %s (Rep %u/%u) =====\n", 
           exp->description, rep_num + 1, exp->repetition);
    printf("[MAIN] Antes do fork - PID: %d, PPID: %d\n", getpid(), getppid());
    
    // Criar um processo filho
    child_pid = fork();
    
    if (child_pid < 0) {
        // Fork falhou
        perror("Fork falhou");
        exit(EXIT_FAILURE);
    } else if (child_pid == 0) {
        // Código do processo filho
        printf("[FILHO] Processo iniciado - PID: %d, PPID: %d\n", getpid(), getppid());
        
        if (exp->wait_child > 0) {
            printf("[FILHO] Dormindo por %d segundos...\n", exp->wait_child);
            
            // Para demonstração de órfãos, verificar o pai periodicamente
            if (strncmp(exp->description, "Experiment D", 12) == 0) {
                sleep(1);
                printf("[FILHO] Após 1s - PID: %d, PPID: %d\n", getpid(), getppid());
                sleep(2);
                printf("[FILHO] Após 3s - PID: %d, PPID: %d\n", getpid(), getppid());
                sleep(2);
                printf("[FILHO] Após 5s - PID: %d, PPID: %d\n", getpid(), getppid());
                sleep(exp->wait_child - 5);
            } else {
                sleep(exp->wait_child);
            }
        }
        
        printf("[FILHO] Processo concluído - PID: %d, PPID: %d\n", getpid(), getppid());
        
        if (exp->wait_post > 0) {
            printf("[FILHO] Sleep pós-execução: %d segundos\n", exp->wait_post);
            sleep(exp->wait_post);
        }
        
        exit(EXIT_SUCCESS);  // Filho termina aqui
    } else {
        // Código do processo pai
        printf("[PAI] Processo continuando - PID: %d, PPID: %d, PID do Filho: %d\n", 
               getpid(), getppid(), child_pid);
        
        if (exp->do_parent_wait_child) {
            // Pai espera o filho terminar
            printf("[PAI] Esperando o filho (PID: %d) terminar...\n", child_pid);
            waitpid(child_pid, &status, 0);
            
            if (WIFEXITED(status)) {
                printf("[PAI] Filho terminou com status: %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("[PAI] Filho terminado pelo sinal: %d\n", WTERMSIG(status));
            }
        } else {
            // Pai continua independentemente
            if (exp->wait_parent > 0) {
                printf("[PAI] Dormindo por %d segundos (não espera pelo filho)...\n", 
                       exp->wait_parent);
                sleep(exp->wait_parent);
            }
        }
        
        printf("[PAI] Processo concluído - PID: %d, PPID: %d\n", getpid(), getppid());
        
        if (exp->wait_post > 0) {
            printf("[PAI] Sleep pós-execução: %d segundos\n", exp->wait_post);
            sleep(exp->wait_post);
        }
        
        // Para o Experimento D, intencionalmente não esperamos pelo filho para demonstrar processos órfãos
        // Para outros experimentos onde o pai não espera explicitamente, garantimos que o filho termine
        if (!exp->do_parent_wait_child && strncmp(exp->description, "Experiment D", 12) != 0) {
            printf("[PAI] Garantindo que o processo filho termine antes do próximo experimento...\n");
            waitpid(child_pid, &status, 0);
        }
        
        printf("[PAI] Experimento totalmente concluído.\n");
    }
}

// Função chamada na saída do programa
void cleanup(void) {
    printf("\nPrograma principal terminando, processos órfãos podem continuar...\n");
    sleep(2); // Tempo para o shell mostrar as saídas do processo órfão
}

int main() {
    // Definir experimentos com inicialização concisa
    struct Experiment experiments[] = {
        { "Experiment A - Sem espera", 2, 5, 1, false, 1 },   // Pai executa independentemente
        { "Experiment B - Pai espera", 0, 3, 1, true, 2 },    // Pai espera pelo filho
        { "Experiment C - Proc rápidos", 0, 0, 0, false, 3 }, // Sem espera
        { "Experiment D - Proc órfãos", 0, 10, 0, false, 1 }  // Intencionalmente cria órfãos
    };
    
    // Registrar o manipulador de saída
    atexit(cleanup);
    
    // Executar cada experimento sequencialmente, com repetições
    size_t num_experiments = sizeof(experiments) / sizeof(struct Experiment);
    
    for (size_t i = 0; i < num_experiments; i++) {
        for (unsigned int r = 0; r < experiments[i].repetition; r++) {
            run_experiment(&experiments[i], r);
            
            // Adicionar um separador entre repetições
            if (r < experiments[i].repetition - 1) {
                printf("\n----- Próxima repetição -----\n");
                sleep(1); // Pequena pausa entre repetições para legibilidade
            }
        }
        
        // Adicionar um separador claro entre experimentos
        printf("\n=========================================\n");
        sleep(1); // Pequena pausa entre experimentos para legibilidade
    }
    
    printf("\nTodos os experimentos concluídos com sucesso.\n");
    return EXIT_SUCCESS;
}