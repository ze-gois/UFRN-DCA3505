/**
 * Producer-Consumer Problem - Mutex-based Solution
 * 
 * This implementation uses a mutex to protect the critical sections
 * where shared resources are accessed.
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

// Mutex for protecting the shared buffer and indices
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

void *produtor(void *arg) {
    int v;
    for (v = 1;; v++) {
        int can_insert = 0;
        
        while (!can_insert) {
            // Acquire the lock to check buffer status
            pthread_mutex_lock(&buffer_mutex);
            
            // Check if there's space in the buffer
            if (((inserir + 1) % TAMANHO) != remover) {
                can_insert = 1;
                
                // Critical section - insert into buffer
                printf("Produzindo %d\n", v);
                dados[inserir] = v;
                inserir = (inserir + 1) % TAMANHO;
            }
            
            // Release the lock
            pthread_mutex_unlock(&buffer_mutex);
            
            if (!can_insert) {
                // If buffer is full, yield and try again later
                usleep(10000);  // Sleep 10ms before trying again
            }
        }
        
        usleep(500000);  // Sleep for 500ms
    }
    
    return NULL;
}

void *consumidor(void *arg) {
    int data;
    size_t consumer_id = (size_t)arg;
    
    for (;;) {
        int can_consume = 0;
        
        while (!can_consume) {
            // Acquire the lock to check buffer status
            pthread_mutex_lock(&buffer_mutex);
            
            // Check if there's data to consume
            if (inserir != remover) {
                can_consume = 1;
                
                // Critical section - consume from buffer
                data = dados[remover];
                printf("Consumidor %zu: Consumindo %d\n", consumer_id, data);
                remover = (remover + 1) % TAMANHO;
            }
            
            // Release the lock
            pthread_mutex_unlock(&buffer_mutex);
            
            if (!can_consume) {
                // If buffer is empty, yield and try again later
                usleep(10000);  // Sleep 10ms before trying again
            }
        }
        
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