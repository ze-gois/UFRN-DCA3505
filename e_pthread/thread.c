#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void* thread(void* arg){
    printf("Olá da thread \n");
    // return NULL;
}

int main(void){
    pthread_t th;
    int rc = pthread_create(&th, NULL, thread, NULL);

    if (rc != 0){
        fprintf(stderr, "Error: não foi possível criar thread: %d\n", rc);
        exit(1);
    }

    pthread_join(th, NULL);

    return 0;
}
