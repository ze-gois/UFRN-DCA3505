#!/bin/bash

echo "===== ANÁLISE DE PROCESSOS ====="
echo "Autor: Sistema Operacional I"
echo "Data: $(date)"
echo -e "\n"

# Parte 1: Execução múltipla do pid_info
echo "===== PARTE 1: EXECUÇÃO MÚLTIPLA DO PID_INFO ====="
echo "Executando o programa pid_info 5 vezes consecutivas:"
echo -e "\n"

for i in {1..5}; do
    echo "Execução $i:"
    ./pid_info
    echo -e "----------------------\n"
done

# Parte 2: Análise da execução do fork_example
echo "===== PARTE 2: ANÁLISE DO FORK_EXAMPLE ====="
echo "Executando o programa fork_example e analisando a relação pai-filho:"
echo -e "\n"

# Executar o fork_example com saída detalhada
./fork_example

# Parte 3: Análise com o comando ps
echo -e "\n===== PARTE 3: ANÁLISE COM O COMANDO PS ====="
echo "Executando fork_example em background e observando com ps:"
echo -e "\n"

# Executa programa fork modificado em background
./fork_example &
FORK_PID=$!

# Aguarda um momento para garantir que o processo filho já foi criado
sleep 1

echo "=> Visualizando a árvore de processos:"
ps f -p $FORK_PID --forest

echo -e "\n=> Informações detalhadas dos processos (format completo):"
ps -f -A | grep -E "fork_example|PID"

echo -e "\n=> Informações de processo por usuário:"
ps -u $USER | grep -E "fork_example|PID"

# Aguarda a conclusão do processo em background
wait $FORK_PID

# Parte 4: Teste com múltiplas chamadas de fork
echo -e "\n===== PARTE 4: TESTE COM FORK MÚLTIPLOS ====="
echo "Agora vamos observar o comportamento de múltiplas chamadas de fork consecutivas:"

echo -e "\nA saída a seguir mostra como os PIDs são atribuídos em chamadas consecutivas:"
for i in {1..3}; do
    echo -e "\nFork #$i:"
    ./fork_example | grep "PID"
done

# Parte 5: Conclusões
echo -e "\n===== PARTE 5: RESUMO E CONCLUSÕES ====="
echo "1. Os PIDs são recursos dinâmicos gerenciados pelo sistema operacional"
echo "2. Cada execução recebe um novo PID, mesmo sendo o mesmo programa"
echo "3. O PPID é o PID do processo que iniciou o programa atual"
echo "4. Após o fork(), temos dois processos com PIDs diferentes executando o mesmo código"
echo "5. O valor retornado por fork() é diferente para o pai (PID do filho) e para o filho (zero)"

# Terminando
echo -e "\n===== FIM DA ANÁLISE ====="
echo "Processo de análise concluído em: $(date)"