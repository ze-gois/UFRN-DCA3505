Tarefa 8: Mutexes

Dada a implementação em linguagem C de um Mutex abaixo:


atomic_bool trava =false;


voidenter_region(void)

{

       bool v;

       do {

               v =false;

       } while (!atomic_compare_exchange_strong(&trava, &v, true));

}


voidleave_region(void)

{

       atomic_store(&trava, false);

}


Responda:
1. Por que a implementação acima não é eficiente quando há contenção?
2. Como a implementação poderia ser minimamente modificada para usar a chamada de sistema futex do Linux? Como essa versão seria melhor que a original?
3. Por que a versão anterior ainda não é ideal do ponto de vista de desempenho?
4. Implemente uma versão que resolve os problemas da questão anterior. Como ela resolve os problemas?

Enviar relatório em PDF com no máximo 4 páginas ou 2000 palavras.

