/**
 * Producer-Consumer Problem - Condition Variables Solution
 * 
 * This implementation uses condition variables to efficiently signal
 * when buffer status changes (not empty/not full).
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define TAMANHO 10
#define NUM_CONSUMIDORES 2
#define RUNTIME_SECONDS 5

volatile int dados[TAMANHO];
volatile size_t inserir = 0;
volatile size_t remover = 0;
volatile int count = 0;  // Number of items in the buffer

// Mutex for protecting the shared buffer and indices
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

// Condition variables for signaling not empty and not full conditions
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

void *produtor(void *arg) {
    int v;
    for (v = 1;; v++) {
        // Acquire the lock
        pthread_mutex_lock(&buffer_mutex);
        
        // Wait while the buffer is full
        while (count == TAMANHO - 1) {
            pthread_cond_wait(&not_full, &buffer_mutex);
        }
        
        // Critical section - insert into buffer
        printf("Produzindo %d\n", v);
        dados[inserir] = v;
        inserir = (inserir + 1) % TAMANHO;
        count++;
        
        // Signal that the buffer is not empty anymore
        pthread_cond_signal(&not_empty);
        
        // Release the lock
        pthread_mutex_unlock(&buffer_mutex);
        
        usleep(500000);  // Sleep for 500ms
    }
    
    return NULL;
}

void *consumidor(void *arg) {
    int data;
    size_t consumer_id = (size_t)arg;
    
    for (;;) {
        // Acquire the lock
        pthread_mutex_lock(&buffer_mutex);
        
        // Wait while the buffer is empty
        while (count == 0) {
            pthread_cond_wait(&not_empty, &buffer_mutex);
        }
        
        // Critical section - consume from buffer
        data = dados[remover];
        printf("Consumidor %zu: Consumindo %d\n", consumer_id, data);
        remover = (remover + 1) % TAMANHO;
        count--;
        
        // Signal that the buffer is not full anymore
        pthread_cond_signal(&not_full);
        
        // Release the lock
        pthread_mutex_unlock(&buffer_mutex);
        
        usleep(rand() % 1000000);  // Random sleep up to 1 second
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