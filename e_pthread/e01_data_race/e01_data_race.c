#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N_THREADS 4
#define N_THREAD_COUNT 10000
int shared_counter = 0;

// Thread function
void* thread_function(void* arg) {
    int id = *(int*)arg;
    int single_counter = 0;

    for (int i = 0; i < N_THREAD_COUNT; i++) {
        shared_counter++;
        single_counter++;
    }

    printf("Thread %d completed with %d single counter and %d shared counter.\n", id, single_counter, shared_counter);
    return 0;
}

int main() {
    pthread_t threads[N_THREADS];
    int thread_ids[N_THREADS];
    int i;

    for (i = 0; i < N_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]) != 0) {
            perror("pthread_create failed");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for threads to complete
    for (i = 0; i < N_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All threads completed. Final counter value: %d\n", shared_counter);

    return 0;
}
