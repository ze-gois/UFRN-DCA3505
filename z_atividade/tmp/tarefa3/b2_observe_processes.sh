#!/bin/bash
#
# b2_observe_processes.sh - Script para observação detalhada de processos
# 
# Este script executa os programas de demonstração em segundo plano
# e captura métricas detalhadas sobre os processos criados.
# 
# Objetivos didáticos:
# 1. Permitir a observação dos processos em tempo real com ferramentas como htop
# 2. Coletar e registrar métricas sobre os processos para análise posterior
# 3. Visualizar a hierarquia entre processos pai e filho em execução
#

# Create logs directory if it doesn't exist
mkdir -p logs

# Clean old logs
rm -f logs/process_*.log

echo "Starting process_id in background..."
./process_id > logs/process_id_output.log &
PID1=$!
echo "Process ID program running with PID: $PID1"

# Capture process info
ps -f -p $PID1 > logs/process_id_info.log
echo "Process info saved to logs/process_id_info.log"

echo "Starting fork_process in background..."
./fork_process > logs/fork_output.log &
PID2=$!
echo "Fork process program running with PID: $PID2"

# Give time for fork to create child
sleep 2

# Use pstree to show the process hierarchy
echo -e "\nProcess tree for fork_process:"
pstree -p $PID2 | tee logs/fork_tree.log

echo -e "\nDetailed process information:"
ps -f --forest | grep -v grep | grep -E "($PID2|$USER)" | tee logs/fork_detailed.log

echo -e "\nProcesses are running in the background."
echo "You now have approximately 25 seconds to use htop to observe them."
echo "To use htop, open another terminal and type:"
echo "  htop"
echo -e "\nIn htop, you can press F3 and search for PID $PID2 to find the fork_process"
echo "Press any key to continue..."
read -n 1

# After user presses a key, collect final stats
echo -e "\nFinal process stats:"
ps -p $PID2 -o pid,ppid,cmd,stat,time | tee -a logs/fork_detailed.log

echo -e "\nWaiting for processes to complete..."
wait

echo -e "\nAll processes completed. Logs saved in the logs directory."
echo "You can examine the logs to see process relationships and behavior."