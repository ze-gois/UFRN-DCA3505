/* number of processes */
#define N   2
#define FALSE   0
#define TRUE    1
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

/* whose turn is it? - declarada como volatile */
volatile int turn;

/* all values initially 0 (FALSE) - declarada como volatile */
volatile int interested[N];

/* Contador compartilhado que será incrementado na região crítica */
volatile int shared_counter = 0;
#define MAX_COUNT 10000

/* process is 0 or 1 */
void enter_region(int process){
    /* number of the other process */
    int other;
    /* the opposite of process */
    other = 1 - process;
    
    /* show that you are interested */
    interested[process] = TRUE;
    /* barreira de memória para garantir que a operação acima seja visível */
    __sync_synchronize();
    
    /* set flag */
    turn = process;
    /* barreira de memória para garantir que a operação acima seja visível */
    __sync_synchronize();
    
    /* null statement com barreira de memória dentro do loop */
    while (1) {
        __sync_synchronize();
        if (!(turn == process && interested[other] == TRUE))
            break;
    }
}

/* process: who is leaving */
void leave_region(int process){
    /* barreira de memória antes de indicar a saída */
    __sync_synchronize();
    /* indicate departure from critical region */
    interested[process] = FALSE;
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
    
    printf("Demonstração do algoritmo de Peterson com barreiras de memória\n");
    printf("Esta versão usa __sync_synchronize() para garantir ordenação de memória\n");
    
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
        printf("Sucesso! O algoritmo funcionou corretamente usando barreiras de memória.\n");
    } else {
        printf("Falha! O algoritmo ainda não garantiu exclusão mútua.\n");
        printf("Talvez seja necessário usar tipos atômicos junto com as barreiras.\n");
    }
    
    return 0;
}
