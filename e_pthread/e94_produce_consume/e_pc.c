#include <stddef.h>
#include <linux/futex.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <stdatomic.h>
#include <sys/syscall.h>
#include <unistd.h>

#define TAMANHO 10
volatile int dados[TAMANHO];
volatile size_t inserir = 0;
volatile size_t remover = 0;

struct THREAD_ARG {
    int id;
};

int shared_counter = 0;
_Atomic uint32_t trava = 0;

void enter_region(void)
{
    uint32_t v = 0;
    if (atomic_compare_exchange_strong_explicit(&trava, &v, 1, memory_order_acquire, memory_order_relaxed)){
        return;
    }

    do {
        if (v == 2 || atomic_compare_exchange_weak_explicit(&trava, &v, 2, memory_order_relaxed, memory_order_relaxed)) {
            syscall(SYS_futex, &trava, FUTEX_WAIT, 2);
        }
        v = 0;
    } while (!atomic_compare_exchange_weak_explicit(&trava, &v, 2, memory_order_acquire, memory_order_relaxed));
}


void leave_region(void) {
    uint32_t v = atomic_fetch_sub_explicit(&trava,1,memory_order_relaxed);
    if ( v != 1 ){
        atomic_store_explicit(&trava, 0, memory_order_relaxed);
        syscall(SYS_futex, &trava, FUTEX_WAKE,1);
    }
}

void *produtor ( void* arg){
    struct THREAD_ARG *a = (struct THREAD_ARG *) arg;
    int v;
    for( v = 1;; v++){
        enter_region();
        while (((inserir + 1)% TAMANHO) == remover){
            leave_region();
            enter_region();
        }
        printf("[%d] Produzindo %d\n",a->id,v);
        dados[inserir] = v;
        inserir = (inserir +1) % TAMANHO;
        leave_region();
        usleep(500000);
    }

    return NULL;
}

void *consumidor(void *arg){
    struct THREAD_ARG *a = (struct THREAD_ARG *) arg;
    for(;;) {
        enter_region();
        while(inserir==remover){
            leave_region();
            enter_region();
        }
        printf("[%d] Consumindo %d\n", a->id, dados[remover]);
        remover = (remover + 1) % TAMANHO;
        leave_region();
    }

    return NULL;
}

#define N_PRODUCERS 1
#define N_CONSUMERS 10

int main (){
    pthread_t id_prod[N_PRODUCERS];
    for (int p = 0; p < N_PRODUCERS; p++){
        struct THREAD_ARG ta = { p };
        pthread_create(&id_prod[p], NULL, produtor, (void *) &ta);
    }

    pthread_t id_cons[N_CONSUMERS];
    for (int c = 0; c < N_CONSUMERS; c++){
        struct THREAD_ARG ta = { c };
        pthread_create(&id_cons[c], NULL, consumidor, (void *) &ta);
    }

    for (int p = 0; p < N_PRODUCERS; p++) pthread_join(id_prod[p],NULL);
        for (int c = 0; c < N_CONSUMERS; c++) pthread_join(id_cons[c],NULL);

    return 0;
}
