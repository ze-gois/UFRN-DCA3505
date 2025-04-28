#include <stdio.h>
#include <stdlib.h>

void g(){
    printf("\n----ALTERED EXECUTION FLOW----\n");
    exit(-1);
}

void f(){
    void *a = NULL;

    *((void**)&a + 2) = g;

    printf("printed\n");
}

int main(){
    f();
    printf("\nFinished main\n");
    return 0;
}
