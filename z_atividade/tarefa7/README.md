Tarefa 7: Algoritmo de Peterson

Dado o algoritmo de Peterson implementado na linguagem C abaixo (extraído de livro de Tanenbaum):

``` C
    #define FALSE 0
    #define TRUE  1
    #define N     2                      /* number of processes */

    int turn;                            /* whose turn is it? */
    int interested[N];                   /* all values initially 0 (FALSE) */

    /* process is in 0..N-1 */
    void enter_region(int process) {
        int other;                  /* number of the other process */
        other = 1 - process;        /* the opposite of process */
        interested[process] = TRUE; /* show that you are interested */
        turn = process;             /* set flag */
        while (turn == process && interested[other] == TRUE) /* null statement */ ;
    }

    void leave_region(int process) {       /* process: who is leaving */
        interested[process] = FALSE;    /* indicate departure from critical region */
    }
```

Responda:

1. Por que usar enter_region e leave_region para estabelecer uma região crítica não funciona com compiladores modernos?
2. Como os problemas descritos na pergunta anterior podem ser resolvidos?
3. Com os problemas dos compiladores modernos resolvidos, qual problema ainda impede que o algoritmo de Peterson funcione em processadores modernos?
4. Como os problemas descritos na pergunta anterior podem ser resolvidos?



Enviar relatório em PDF com no máximo 4 páginas ou 2000 palavras.
