#!/bin/bash

echo "Iniciando script para observar processos..."
echo "Executando fork_example em segundo plano..."

# Executa o programa fork_example em segundo plano
./fork_example &

# Captura o PID do processo pai (fork_example)
PARENT_PID=$!
echo "PID do processo pai (fork_example): $PARENT_PID"

# Espera um pouco para garantir que o processo filho já foi criado
sleep 1

echo -e "\n--- Listagem de processos relacionados ---"
# Mostra os processos em formato de árvore, destacando o processo pai e filho
ps f -p $PARENT_PID --forest

echo -e "\n--- Listagem de processos em formato detalhado ---"
ps -f -p $PARENT_PID --forest

echo -e "\n--- Listagem de todos os processos do usuário atual ---"
ps -u $USER | grep -E "fork_example|PID"

echo -e "\nAguardando a finalização dos processos..."
wait $PARENT_PID
echo "Processos finalizados."