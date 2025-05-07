#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <stdint.h>
#include <stdatomic.h>
#include <pthread.h>

// Define futex system call wrapper
static int futex(uint32_t *uaddr, int futex_op, uint32_t val,
                const struct timespec *timeout, uint32_t *uaddr2, uint32_t val3) {
    return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
}

// Our mutex structure
typedef struct {
    atomic_uint state;  // 0 = unlocked, 1 = locked
} mufutex_t;

// Initialize mutex
int mufutex_init(mufutex_t *mutex) {
    if (!mutex)
        return -EINVAL;

    atomic_store(&mutex->state, 0);  // Initialize to unlocked state
    return 0;
}

// Lock mutex
int mufutex_lock(mufutex_t *mutex) {
    uint32_t zero = 0;

    // Try to atomically change state from 0 (unlocked) to 1 (locked)
    while (!atomic_compare_exchange_strong(&mutex->state, &zero, 1)) {
        // If already locked, wait using futex
        futex((uint32_t *)&mutex->state, FUTEX_WAIT, 1, NULL, NULL, 0);
        zero = 0;  // Reset for next attempt
    }

    return 0;
}

// Unlock mutex
int mufutex_unlock(mufutex_t *mutex) {
    uint32_t one = 1;

    // Try to atomically change state from 1 (locked) to 0 (unlocked)
    if (!atomic_compare_exchange_strong(&mutex->state, &one, 0)) {
        // If it wasn't locked, that's an error
        return -EPERM;
    }

    // Wake up one waiting thread
    futex((uint32_t *)&mutex->state, FUTEX_WAKE, 1, NULL, NULL, 0);
    return 0;
}

// Global mutex and counter for demonstration
mufutex_t mutex;
int shared_counter = 0;

// Thread function
void* thread_function(void* arg) {
    int id = *(int*)arg;
    int i;

    for (i = 0; i < 30; i++) {
        // Lock the mutex
        printf("Thread %d trying to lock mutex\n", id);
        mufutex_lock(&mutex);

        // Critical section
        printf("Thread %d entered critical section, counter = %d\n", id, shared_counter);
        shared_counter++;

        // // Simulate some work
        // usleep(100000);  // Sleep for 100ms

        printf("Thread %d leaving critical section, counter = %d\n", id, shared_counter);

        // Unlock the mutex
        mufutex_unlock(&mutex);
        printf("Thread %d released mutex\n", id);

        // // Non-critical section - sleep a bit
        // usleep(50000);  // Sleep for 50ms
    }

    return NULL;
}

int main() {
    pthread_t threads[4];
    int thread_ids[4];
    int i;

    // Initialize mutex
    mufutex_init(&mutex);
    printf("Mutex initialized\n");

    // Create threads
    for (i = 0; i < 4; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]) != 0) {
            perror("pthread_create failed");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for threads to complete
    for (i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All threads completed. Final counter value: %d\n", shared_counter);

    return 0;
}
