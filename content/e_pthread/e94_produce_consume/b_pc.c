#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define TAMANHO 10
volatile int dados[TAMANHO];
volatile size_t inserir = 0;
volatile size_t remover = 0;

void *produtor ( void* arg){
    int v;
    for( v = 1;; v++){
        while (((inserir + 1)% TAMANHO) == remover);
        printf("Produzindo %d\n",v);
        dados[inserir] = v;
        inserir = (inserir +1) % TAMANHO;

        usleep(500000);
    }

    return NULL;
}

void *consumidor(void *arg){
    for(;;) {
        while(inserir==remover);
        printf("%zu: Consumindo %d\n", (size_t) arg, dados[remover]);
        remover = (remover + 1) % TAMANHO;
    }

    return NULL;
}

int main (){
    pthread_t pt_id[3];

    pthread_create(&pt_id[0], NULL, produtor, (void *) 0);
    pthread_create(&pt_id[1], NULL, consumidor, (void *) 1);
    pthread_create(&pt_id[2], NULL, consumidor, (void *) 2);

    for (int p=0; p< 2 ; p++){
        pthread_join(pt_id[p], NULL);
    }

    return 0;
}
