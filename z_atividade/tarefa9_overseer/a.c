struct A {
    void * b;
};

int main(){
    struct A *a = { struct {
        uint8_t c = 3;
    } };

    printf("%d",a.b.c);
    return 0;
}
