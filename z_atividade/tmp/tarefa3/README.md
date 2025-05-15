# Tarefa 3: Criação e identificação de processos

Este projeto demonstra a criação e identificação de processos no Linux usando funções como `getpid()`, `getppid()` e `fork()`.

## Estrutura de Arquivos

Os arquivos estão organizados com um sistema de nomenclatura didático para facilitar o aprendizado:

### Sistema de Nomenclatura

* O prefixo alfabético (`a`, `b`, `c`) indica a categoria do arquivo:
  - `a`: Código-fonte (implementações em C)
  - `b`: Scripts de execução e observação
  - `c`: Documentação e relatórios

* O número após a letra indica a ordem sugerida de estudo/uso dentro da categoria

Este sistema de nomenclatura permite que um estudante novo no projeto entenda facilmente qual a sequência lógica de estudo dos arquivos.

### Arquivos do Projeto

- Prefixo `a*`: Arquivos fonte C
  - `a1_process_id.c`: Demonstração de identificação de processos
  - `a2_fork_process.c`: Demonstração de criação de processos com fork
  
- Prefixo `b*`: Scripts de execução
  - `b1_run_all.sh`: Executa os programas sequencialmente
  - `b2_observe_processes.sh`: Executa os programas em background para observação
  
- Prefixo `c*`: Documentação
  - `c1_relatorio.tex`: Template do relatório
  - `c2_compile_report.sh`: Script para compilar o relatório

## Compilação

Para compilar os programas, execute:

```
make
```

Isso irá gerar dois executáveis: `process_id` e `fork_process`.

## Programas

### 1. a1_process_id.c → process_id

Este programa exibe o identificador do processo atual (PID) e o identificador do processo pai (PPID). O programa aguarda 15 segundos após exibir as informações para permitir a observação do processo.

Para executar:

```
./process_id
```

Execute várias vezes para observar se os valores mudam entre as execuções.

### 2. a2_fork_process.c → fork_process

Este programa cria um novo processo usando `fork()`. Tanto o processo pai quanto o filho imprimem suas respectivas identificações. Os processos aguardam 30 segundos após a execução para permitir a observação com ferramentas como `htop`.

Para executar:

```
./fork_process
```

## Observação dos Processos

Para facilitar a observação dos processos em execução, este projeto inclui dois scripts:

### b1_run_all.sh

Executa os programas sequencialmente e exibe informações básicas:

```
./b1_run_all.sh
```

### b2_observe_processes.sh

Executa os programas em segundo plano e captura métricas detalhadas sobre eles:

```
./b2_observe_processes.sh
```

Este script:
1. Executa os programas em segundo plano
2. Captura informações detalhadas sobre os processos
3. Dá tempo suficiente para o usuário observar os processos com ferramentas como `htop`
4. Salva logs com as informações dos processos no diretório `logs/`

Durante a execução do script, você pode usar comandos como `ps`, `top` ou `htop` em outro terminal para visualizar os processos em execução:

```
ps -f
```

ou

```
htop
```

## Relatório

Um template para o relatório está disponível em `c1_relatorio.tex`. Para compilá-lo, execute:

```
./c2_compile_report.sh
```

## Limpeza

Para limpar os executáveis gerados:

```
make clean
```