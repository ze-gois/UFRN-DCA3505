struct esperando
{
    struct mutex m;
    struct esperando *prox;
};

struct semaforo
{
    struct mutex trava;
    size_t valor;
    struct esperando *formador;
    struct esperando *terminal;
}

void sem_inicializar(struct semaforo *s)
{
    inicializar(&s->trava);
    s->valor = 0;
    s->formador = NULL;
    s->terminal = NULL;
}

void sem_incrementar(struct semaforo *s)
{

}

void sem_decrementar(struct semaforo *s)
{

}

void *produtor(void *arg){

}

void *consumidor(void *arg){

}
