#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#define TAMANHO 10

volatile int dados[TAMANHO];
volatile size_t inserir = 0;
volatile size_t remover = 0;

void *produtor(void *arg){
    int v;

    for(v=1;; v++){
        while(((inserir +1) % TAMANHO) == remover);
        printf("Produzindo %d\n",v);
        dados[inserir] = v;
        inserir = (inserir + 1) % TAMANHO;
        usleep(500000);
    }

    return NULL;
}
