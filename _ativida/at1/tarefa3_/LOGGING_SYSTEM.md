# Sistema de Logging para Experimentos de Processos

Este documento descreve o sistema de logging implementado para análise detalhada da criação, comportamento e finalização de processos, com foco especial na oscilação de PIDs.

## Visão Geral

O sistema de logging foi projetado para capturar métricas precisas sobre o comportamento dos processos em um ambiente Linux, incluindo:

- Rastreamento de PIDs ao longo do tempo
- Medição de incrementos entre PIDs consecutivos
- Registro do ciclo de vida completo dos processos
- Coleta de estatísticas para análise posterior

## Estrutura do Sistema

### Diretório de Logs

Todos os logs são armazenados no diretório `logs/` com a seguinte estrutura:

- `<nome_experimento>_<timestamp>.log` - Log de texto legível
- `<nome_experimento>_<timestamp>.csv` - Dados estruturados para análise
- `<nome_experimento>_<timestamp>_analysis.txt` - Análise automática
- `<nome_experimento>_<timestamp>_summary.txt` - Resumo do experimento

### Componentes Principais

1. **log_utils.sh**: Biblioteca de funções para logging padronizado
2. **spawn_processes_logged.sh**: Script para criar processos com logging
3. **kill_random_logged.sh**: Script para terminar processos com logging
4. **pid_oscillation_experiment.sh**: Experimento completo de oscilação de PIDs

## Formatos de Log

### Formato CSV

O arquivo CSV contém os seguintes campos:

```
timestamp,event_type,pid,ppid,command,duration_ms,exit_code,additional_info
```

- **timestamp**: Data e hora com precisão de milissegundos
- **event_type**: Tipo de evento (CREATED, TERMINATED, INFO)
- **pid**: ID do processo
- **ppid**: ID do processo pai
- **command**: Comando executado
- **duration_ms**: Duração da operação em milissegundos
- **exit_code**: Código de saída (para eventos de término)
- **additional_info**: Informações contextuais adicionais

### Arquivo de Análise

O sistema gera automaticamente análises incluindo:
- Estatísticas de incremento de PIDs
- Duração de vida dos processos
- Relações pai-filho observadas
- Padrões de oscilação de PIDs

## Como Usar o Sistema

### Executando Experimentos Pré-configurados

```bash
# Experimento completo de oscilação de PIDs
make pid_oscillation_logged

# Apenas criação de processos com logging
make spawn_logged

# Apenas finalização de processos com logging
make kill_logged
```

### Usando as Funções de Logging em Scripts Personalizados

```bash
# Importar a biblioteca de logging
source ./log_utils.sh

# Iniciar um experimento
init_experiment "meu_experimento" 

# Registrar mensagens informativas
log_message "Iniciando teste..."

# Registrar criação de processo
log_process_created "$pid" "$ppid" "$comando" "informação adicional"

# Registrar fim de processo
log_process_terminated "$pid" "$ppid" "$comando" "$duracao" "$codigo_saida" "info adicional"

# Registrar informação de PID
log_pid_info "$pid" "$ppid" "contexto descritivo"

# Finalizar o experimento (gera sumário)
finish_experiment

# Gerar análise detalhada
analyze_experiment "$CURRENT_EXPERIMENT"
```

## Analisando os Resultados

### Visualização no Terminal

Os arquivos de log podem ser examinados diretamente:

```bash
# Ver log de texto
cat logs/<experimento>_<timestamp>.log

# Ver resumo do experimento
cat logs/<experimento>_<timestamp>_summary.txt

# Ver análise detalhada
cat logs/<experimento>_<timestamp>_analysis.txt
```

### Análise de Dados

O formato CSV facilita a análise em ferramentas como:
- Excel/LibreOffice Calc
- Python (pandas)
- R

Exemplo de gráfico que pode ser gerado:
- Evolução dos valores de PID ao longo do tempo
- Histograma de incrementos entre PIDs consecutivos
- Duração da vida dos processos

## Benefícios do Sistema

1. **Precisão**: Registro com timestamps precisos
2. **Consistência**: Formato padronizado para todos os eventos
3. **Rastreabilidade**: Acompanhamento completo do ciclo de vida
4. **Análise**: Ferramentas automatizadas para extrair insights

## Métricas Capturadas

O sistema captura diversas métricas importantes:

1. **Taxa de incremento de PIDs**: Como os PIDs aumentam em diferentes situações
2. **Comportamento após criação/destruição**: Impacto no próximo PID atribuído
3. **Variabilidade**: Quanto os incrementos de PID variam entre execuções
4. **Correlação com atividade**: Relação entre número de processos e saltos de PID

## Conclusão

Este sistema de logging fornece uma estrutura robusta para experimentos com processos em Linux, permitindo compreender melhor o comportamento do sistema operacional na gestão de processos e alocação de PIDs.