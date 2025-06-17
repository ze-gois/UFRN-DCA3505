/* number of processes */
#define N   2
#define FALSE   0
#define TRUE    1
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

/* whose turn is it? - usando tipo atômico do C11 */
atomic_int turn;

/* all values initially 0 (FALSE) - usando tipo atômico do C11 */
atomic_int interested[N];

/* Contador compartilhado que será incrementado na região crítica */
atomic_int shared_counter;
#define MAX_COUNT 10000

/* process is 0 or 1 */
void enter_region(int process){
    /* number of the other process */
    int other;
    /* the opposite of process */
    other = 1 - process;
    
    /* show that you are interested - usando API atômica do C11 */
    atomic_store_explicit(&interested[process], TRUE, memory_order_seq_cst);
    
    /* set flag - usando API atômica do C11 */
    atomic_store_explicit(&turn, process, memory_order_seq_cst);
    
    /* null statement - usando API atômica do C11 para leitura */
    while (atomic_load_explicit(&turn, memory_order_seq_cst) == process && 
           atomic_load_explicit(&interested[other], memory_order_seq_cst) == TRUE);
}

/* process: who is leaving */
void leave_region(int process){
    /* indicate departure from critical region - usando API atômica do C11 */
    atomic_store_explicit(&interested[process], FALSE, memory_order_seq_cst);
}

/* Função que será executada pelas threads */
void* process_function(void* arg) {
    int process_id = *(int*)arg;
    
    for (int i = 0; i < MAX_COUNT; i++) {
        enter_region(process_id);
        
        // Região crítica - incrementar o contador compartilhado
        // Usando operação atômica C11 para incremento
        atomic_fetch_add_explicit(&shared_counter, 1, memory_order_seq_cst);
        
        leave_region(process_id);
    }
    
    printf("Processo %d terminou\n", process_id);
    return NULL;
}

int main() {
    pthread_t threads[N];
    int process_ids[N];
    
    printf("Demonstração do algoritmo de Peterson com biblioteca atômica C11\n");
    printf("Esta versão usa stdatomic.h para garantir atomicidade e ordenação de memória\n");
    
    // Inicializar valores
    atomic_store(&turn, 0);
    atomic_store(&interested[0], FALSE);
    atomic_store(&interested[1], FALSE);
    atomic_store(&shared_counter, 0);
    
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
    int final_counter = atomic_load(&shared_counter);
    printf("Valor final do contador: %d\n", final_counter);
    printf("Valor esperado: %d\n", N * MAX_COUNT);
    
    if (final_counter == N * MAX_COUNT) {
        printf("Sucesso! O algoritmo funcionou corretamente usando atomicidade C11.\n");
    } else {
        printf("Falha! O algoritmo ainda não garantiu exclusão mútua.\n");
    }
    
    return 0;
}
