#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

uint64_t valor = 0;

void* thread(void* arg) {
    size_t i = 1000000;
    while (i--) {
        valor++;
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;
    
    // Criar duas threads
    pthread_create(&t1, NULL, thread, NULL);
    pthread_create(&t2, NULL, thread, NULL);
    
    // Aguardar as threads terminarem
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    // Imprimir o resultado
    printf("Valor final: %lu\n", valor);
    
    return 0;
}