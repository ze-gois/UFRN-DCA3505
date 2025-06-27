/**
 * Producer-Consumer Problem - Semaphore-based Solution
 * 
 * This implementation uses semaphores to synchronize access to the shared buffer.
 * Two semaphores are used:
 * - empty: counts the number of empty slots in the buffer
 * - full: counts the number of filled slots in the buffer
 * A mutex is still used to protect critical sections.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#define TAMANHO 10
#define NUM_CONSUMIDORES 2
#define RUNTIME_SECONDS 5

volatile int dados[TAMANHO];
volatile size_t inserir = 0;
volatile size_t remover = 0;

// Mutex for protecting the buffer operations
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

// Semaphore for empty slots (initially all slots are empty)
sem_t empty_slots;

// Semaphore for filled slots (initially no slots are filled)
sem_t filled_slots;

void *produtor(void *arg) {
    int v;
    for (v = 1;; v++) {
        // Wait for an empty slot
        sem_wait(&empty_slots);
        
        // Acquire the mutex to protect the critical section
        pthread_mutex_lock(&buffer_mutex);
        
        // Critical section - insert into buffer
        printf("Produzindo %d\n", v);
        dados[inserir] = v;
        inserir = (inserir + 1) % TAMANHO;
        
        // Release the mutex
        pthread_mutex_unlock(&buffer_mutex);
        
        // Signal that a new item is available
        sem_post(&filled_slots);
        
        usleep(500000);  // Sleep for 500ms
    }
    
    return NULL;
}

void *consumidor(void *arg) {
    int data;
    size_t consumer_id = (size_t)arg;
    
    for (;;) {
        // Wait for a filled slot
        sem_wait(&filled_slots);
        
        // Acquire the mutex to protect the critical section
        pthread_mutex_lock(&buffer_mutex);
        
        // Critical section - consume from buffer
        data = dados[remover];
        printf("Consumidor %zu: Consumindo %d\n", consumer_id, data);
        remover = (remover + 1) % TAMANHO;
        
        // Release the mutex
        pthread_mutex_unlock(&buffer_mutex);
        
        // Signal that a new empty slot is available
        sem_post(&empty_slots);
        
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
    
    // Initialize semaphores
    sem_init(&empty_slots, 0, TAMANHO - 1);  // Buffer size - 1 empty slots initially
    sem_init(&filled_slots, 0, 0);  // 0 filled slots initially
    
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