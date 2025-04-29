#include <stdio.h>
#include <stdlib.h>

void g(){
    printf("alterado\n");
    exit(-1);
}

void f(){
    void *a = NULL;
    *(&a + 3) = g;
    printf("printed\n");
}

int main(){
    f();
    printf("\nFinished main\n");
    return 0;
}
