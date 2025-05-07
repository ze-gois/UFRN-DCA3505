#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/syscall.h>
#include <linux/futex.h>

uint64_t valor = 0;
_Atomic uint32_t trava = 0;

void enter_region(void){
    uint32_t v;
    do {
        syscall(SYS_futex, &trava, FUTEX_WAIT,1 );
        v = atomic_exchange(&trava, 1);
    } while(v);
}

typedef struct {
    int x;
    int id;
    char name[10];
} NossaEstrutura;

void* thread(void* arg){
    NossaEstrutura* data = (NossaEstrutura*)arg;
    printf("Thread %d (%s,%d) iniciada\n", data->id, data->name, data->x);

    for (int m=0; m < 1000000; m++){
        enter_region();
        counter += 1;
        leave_region();
    }
}

void leave_region(void) {
    atomic_store(&trava, false);
    syscall(SYS_futex, &trava, FUTEX_WAKE, 1);
}

void pcreated_right(int rc) {
    if (rc != 0){
        fprintf(stderr, "Error: não foi possível criar thread: %d\n", rc);
        exit(1);
    }
}

int main(void){
    pthread_t th1;
    pthread_t th2;

    NossaEstrutura data1 = {-3, 0, "T1"};
    NossaEstrutura data2 = {62, 1, "T2"};

    int rc1 = pthread_create(&th1, NULL, thread, &data1);
    int rc2 = pthread_create(&th2, NULL, thread, &data2);

    pcreated_right(rc1);
    pcreated_right(rc2);

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);

    printf("counter= %d", counter);

    return 0;
}
int main() {
    // FUTEX_WAIT
}
