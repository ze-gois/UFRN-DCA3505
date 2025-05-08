#include <linux/futex.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <stdatomic.h>
#include <sys/syscall.h>
#include <unistd.h>

#define N_THREADS 4
#define N_THREAD_COUNT 1000000
int shared_counter = 0;
_Atomic uint32_t trava = 0;

void enter_region(void)
{
    uint32_t v = 0;
    if (atomic_compare_exchange_strong(&trava, &v, 1)){
        return;
    }

    do {
        if (v == 2 || atomic_compare_exchange_strong(&trava, &v, 2)) {
            syscall(SYS_futex, &trava, FUTEX_WAIT, 2);
        }
        v = 0;
    } while (!atomic_compare_exchange_strong(&trava, &v, 2));
}


void leave_region(void) {
    uint32_t v = atomic_fetch_sub(&trava,1);
    if ( v!=1 ){
        atomic_store(&trava, 0);
        syscall(SYS_futex, &trava, FUTEX_WAKE,1);
    }
}

// Thread function
void* thread_function(void* arg) {
    int id = *(int*)arg;
    int single_counter = 0;

    for (int i = 0; i < N_THREAD_COUNT; i++) {
        enter_region();
        shared_counter++;
        leave_region();
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
