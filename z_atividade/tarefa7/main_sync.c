/* number of processes */
#define N   2
#define FALSE   0
#define TRUE    1
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

/* Substituindo o algoritmo de Peterson por mutex POSIX */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* Contador compartilhado que será incrementado na região crítica */
int shared_counter = 0;
#define MAX_COUNT 10000

/* Função de entrada substituída por lock de mutex */
void enter_region(int process){
    pthread_mutex_lock(&mutex);
}

/* Função de saída substituída por unlock de mutex */
void leave_region(int process){
    pthread_mutex_unlock(&mutex);
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
    
    printf("Demonstração usando mutex POSIX em vez do algoritmo de Peterson\n");
    printf("Esta versão substitui completamente o algoritmo por mutexes\n");
    
    // Inicializar o contador
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
        printf("Sucesso! O mutex POSIX garantiu exclusão mútua corretamente.\n");
        printf("Esta é a abordagem recomendada em sistemas modernos.\n");
    } else {
        printf("Falha! Houve um problema com o mutex.\n");
    }
    
    // Destruir o mutex
    pthread_mutex_destroy(&mutex);
    
    return 0;
}
