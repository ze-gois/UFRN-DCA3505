Tarefa 9: Produtor e consumidor

Dadas as função de produtor e consumidor em linguagem C abaixo:


#define TAMANHO 10

volatileintdados[TAMANHO];

volatilesize_t inserir =0;

volatilesize_t remover =0;


void*produtor(void*arg)

{

       int v;

       for (v =1;; v++) {

               while (((inserir +1) % TAMANHO) == remover);

               printf("Produzindo %d\n", v);

               dados[inserir] = v;

               inserir = (inserir +1) % TAMANHO;

               usleep(500000);

       }


       returnNULL;

}


void*consumidor(void*arg)

{

       for (;;) {

               while (inserir == remover);

               printf("%zu: Consumindo %d\n", (size_t)arg, dados[remover]);

               remover = (remover +1) % TAMANHO;

       }


       returnNULL;

}


Responda:

1. Qual é o problema fundamental da implementação acima?
2. Como a implementação pode ser modificada para resolver o problema usando um ou mais Mutexes?
3. A solução apenas com Mutexes tem problemas de desempenho. Quais são esses problemas?
4. Como esses problemas podem ser resolvidos com semáforos?
5. Como esses problemas podem ser resolvidos com variáveis de condição?

Enviar relatório em PDF com no máximo 4 páginas ou 2000 palavras.
