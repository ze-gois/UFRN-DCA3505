#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

#define FALSE 0
#define TRUE 1
#define N 2

int counter = 0;
pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER;

int turn;
int interested[N];

typedef struct {
    int x;
    int id;
    char name[10];
} NossaEstrutura;


void enter_region(int this_process){
    int other_process = 1-this_process;
    interested[this_process] = TRUE;
    turn = this_process;
    // asm volatile("" ::: "memory");
    // asm volatile("mfence" ::: "memory");
    while (interested[other_process] == TRUE && turn == this_process);
}

void* thread(void* arg){
    NossaEstrutura* data = (NossaEstrutura*)arg;
    printf("Thread %d (%s,%d) iniciada\n", data->id, data->name, data->x);

    for (int m=0; m < 1000000; m++){
        pthread_mutex_lock(&mutex);
        counter += 1;
        pthread_mutex_unlock(&mutex);
    }
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
