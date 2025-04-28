#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

_Atomic int counter = 0;

void* thread(void* arg){
    for (int m=0; m <= 1000000; m++)
    counter += 1;
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

    int rc1 = pthread_create(&th1, NULL, thread, NULL);
    int rc2 = pthread_create(&th2, NULL, thread, NULL);

    pcreated_right(rc1);
    pcreated_right(rc2);

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);

    printf("counter= %d", counter);

    return 0;
}
