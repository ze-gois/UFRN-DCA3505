/* number of processes */
#define N   2
#define FALSE   0
#define TRUE    1
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

/* whose turn is it? */
int turn;

/* all values initially 0 (FALSE) */
int interested[N];

/* Contador compartilhado que será incrementado na região crítica */
int shared_counter = 0;
#define MAX_COUNT 10000000

/* process is 0 or 1 */
void enter_region(int process){
    /* number of the other process */
    int other;
    /* the opposite of process */
    other = 1 - process;
    
    /* show that you are interested - usando operação atômica */
    __atomic_store_n(&interested[process], TRUE, __ATOMIC_SEQ_CST);
    
    /* set flag - usando operação atômica */
    __atomic_store_n(&turn, process, __ATOMIC_SEQ_CST);
    
    /* null statement - usando operações atômicas para leitura */
    int other_interested, current_turn;
    do {
        __atomic_load(&interested[other], &other_interested, __ATOMIC_SEQ_CST);
        __atomic_load(&turn, &current_turn, __ATOMIC_SEQ_CST);
    } while (current_turn == process && other_interested == TRUE);
}

/* process: who is leaving */
void leave_region(int process){
    /* indicate departure from critical region - usando operação atômica */
    __atomic_store_n(&interested[process], FALSE, __ATOMIC_SEQ_CST);
}

/* Função que será executada pelas threads */
void* process_function(void* arg) {
    int process_id = *(int*)arg;
    
    for (int i = 0; i < MAX_COUNT; i++) {
        enter_region(process_id);
        
        // Região crítica - incrementar o contador compartilhado
        // Usando operação atômica para incremento
        __atomic_fetch_add(&shared_counter, 1, __ATOMIC_SEQ_CST);
        
        leave_region(process_id);
    }
    
    printf("Processo %d terminou\n", process_id);
    return NULL;
}

int main() {
    pthread_t threads[N];
    int process_ids[N];
    
    printf("Demonstração do algoritmo de Peterson com operações atômicas GCC\n");
    printf("Esta versão usa __atomic_store_n e __atomic_load para garantir atomicidade\n");
    
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
        printf("Sucesso! O algoritmo funcionou corretamente usando operações atômicas.\n");
    } else {
        printf("Falha! O algoritmo ainda não garantiu exclusão mútua.\n");
    }
    
    return 0;
}
