#include <stdio.h>

void g(){
    printf("altered");
}

void f(){
    void * a = NULL;
    printf("printed");
    //smth here
    *((void**)&a -4) = g;
}

int main(){
    f();
    printf("main");
}
