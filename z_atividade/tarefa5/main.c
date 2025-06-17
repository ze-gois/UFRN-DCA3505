#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
// #include <stddef.h>

#define N_THREADS 2
#define N_THREAD_COUNT 1000000
uint64_t valor = 0;

struct TA {
    size_t id;
    size_t c;
};

void* thread_function(void* arg) {
    struct TA *ta = (struct TA *) (arg);
    size_t i = N_THREAD_COUNT;
    while (i--) {
        ta->c++;
        valor++;
        printf("%zu -> (%zu,%zu)\n ",ta->id, ta->c, valor);
    }

    free(ta);
    return NULL;
}

int main() {
    int result;
    pthread_t thread_identifier[N_THREADS];

    for (size_t t = 0; t < N_THREADS; t++){
        struct TA *thread_argument = malloc(sizeof(struct TA));
        thread_argument->c = 0;
        thread_argument->id = t;

        result = pthread_create(thread_identifier+t, NULL, thread_function, (void *) thread_argument);

        if (result != 0) {
            fprintf(stderr, "Error creating thread %zu: %d\n", t, result);
            exit(2);
        }
    }

    for (size_t t = 0; t < N_THREADS; t++) {
        result = pthread_join(thread_identifier[t], NULL);
        if (result != 0) {
            fprintf(stderr, "Error joining thread %zu: %d\n", t, result);
            exit(3);
        }
    }

    printf("All threads completed. Final counter value: %zu\n", valor);

    return 0;
}
