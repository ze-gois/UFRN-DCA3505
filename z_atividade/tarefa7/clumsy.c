/* number of processes */
#define N   2
#define FALSE   0
#define TRUE    1
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdatomic.h>

/* whose turn is it? */
int turn;

/* all values initially 0 (FALSE) */
int interested[N];

/* Contador compartilhado que será incrementado na região crítica */
int shared_counter = 0;
#define MAX_COUNT 10000000

/* process is 0 or 1 */
void enter_region(int process_id){
    /* the opposite of process */
    int other = 1 - process_id;
    /* show that you are interested */
    interested[process_id] = TRUE;
    /* set flag */
    turn = process_id;

    /* null statement com timeout para evitar deadlock */
    time_t start_time = time(NULL);
    while (turn == process_id && interested[other] == TRUE) {
        /* Verificar timeout após 2 segundos para evitar spinlock infinito */
        if (time(NULL) - start_time > 2) {
            printf("Timeout no processo %d! Isso demonstra o problema com otimizações.\n", process_id);
            break;
        }
    }
}

/* process: who is leaving */
void leave_region(int process_id){
    /* indicate departure from critical region */
    interested[process_id] = FALSE;
}

/* Função que será executada pelas threads */
void* process_function(void* arg) {
    int process_id = *(int*)arg;

    for (int i = 0; i < MAX_COUNT; i++) {
        enter_region(process_id);

        // Região crítica - incrementar o contador compartilhado
        shared_counter++;

        leave_region(process_id);
    }

    printf("Processo %d terminou\n", process_id);
    return NULL;
}

int main() {
    pthread_t threads[N];
    int process_ids[N];

    printf("Demonstração do algoritmo de Peterson (versão original COM otimizações)\n");
    printf("Este exemplo poderá falhar devido a otimizações do compilador e reordenamento de memória\n");

    // Inicializar valores
    turn = 0;
    interested[0] = FALSE;
    interested[1] = FALSE;
    shared_counter = 0;

    // Criar threads
    for (int i = 0; i < N; i++) {
        process_ids[i] = i;
        if (pthread_create(&threads[i], NULL, process_function, &process_ids[i]) != 0) {
            printf("Erro ao criar thread %d\n", i);
            return 1;
        }
    }

    // Aguardar threads terminarem
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    // Verificar resultado
    printf("Valor final do contador: %d\n", shared_counter);
    printf("Valor esperado: %d\n", N * MAX_COUNT);

    if (shared_counter == N * MAX_COUNT) {
        printf("Sucesso! O algoritmo funcionou corretamente.\n");
        printf("Isto é surpreendente com as otimizações ativadas!\n");
    } else {
        printf("Falha! O algoritmo não garantiu exclusão mútua.\n");
        printf("Isso demonstra os problemas com compiladores e processadores modernos.\n");
    }

    return 0;
}
