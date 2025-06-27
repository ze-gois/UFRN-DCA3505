/**
 * Producer-Consumer Problem - Original Implementation with Race Conditions
 *
 * This implementation demonstrates the race conditions that occur when
 * shared resources are accessed without proper synchronization.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define TAMANHO 10
#define NUM_CONSUMIDORES 2
#define RUNTIME_SECONDS 25

volatile int dados[TAMANHO];
volatile size_t inserir = 0;
volatile size_t remover = 0;

void *produtor(void *arg) {
    int v;
    for (v = 1;; v++) {
        // Busy wait until there's space in the buffer
        while (((inserir + 1) % TAMANHO) == remover);

        printf("Produzindo %d\n", v);
        dados[inserir] = v;
        inserir = (inserir + 1) % TAMANHO;
        usleep(500000); // Sleep for 500ms
    }
    return NULL;
}

void *consumidor(void *arg) {
    for (;;) {
        // Busy wait until there's data to consume
        while (inserir == remover);

        printf("Consumidor %zu: Consumindo %d\n", (size_t)arg, dados[remover]);
        remover = (remover + 1) % TAMANHO;
        usleep(rand() % 1000000); // Random sleep up to 1 second
    }
    return NULL;
}

int main() {
    pthread_t prod_thread;
    pthread_t cons_threads[NUM_CONSUMIDORES];
    size_t i;

    // Seed the random number generator
    srand(time(NULL));

    // Create producer thread
    pthread_create(&prod_thread, NULL, produtor, NULL);

    // Create consumer threads
    for (i = 0; i < NUM_CONSUMIDORES; i++) {
        pthread_create(&cons_threads[i], NULL, consumidor, (void *)i);
    }

    // Run for a few seconds
    printf("Running for %d seconds...\n", RUNTIME_SECONDS);
    sleep(RUNTIME_SECONDS);

    printf("Program finished. Note: In a real application, we would properly join threads.\n");
    printf("For demonstration purposes, we're exiting directly to avoid handling thread cancellation.\n");

    return 0;
}
