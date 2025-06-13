Tarefa 5: Concorrência com threads

Dado o trecho abaixo em linguagem C:

uint64_t valor =0;

void*thread(void*arg)

{

       size_t i =1000000;

       while (i--) {

               valor++;

       }

}

Adicione ao programa acima uma função main que crie duas threads que executam a função acima concorrentemente e espere que elas terminem; ao final, imprima na tela o valor da variável valor.

Responda:

1. Por que, quando compilado com gcc nas suas configurações padrões, o resultado não é 2 milhões como é de se esperar?
2. Por que o resultado é 2 milhões quando habilitamos as otimizações do gcc?
3. Com as otimizações habilitadas, seria possível o valor final ser menos de 2 milhões? Por quê?
4. Descreva uma forma de implementar a função acima que resolve o problema da contagem errada. Por que ela resolve o problema?

 

Enviar relatório em PDF com no máximo 4 páginas ou 2000 palavras.
