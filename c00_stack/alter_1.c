#include <stdint.h>
#include <stdio.h>

void g(){
    printf("altered");
}

void f(){
    printf("printed");
    *((void (*) (void))(((uint64_t)&f)-4)) = &g;
    // *((void (*) (void))(((uint64_t)&f)-4)) = (void(*)(void))(&g);
}

int main(){
    f();
    printf("main");
}
