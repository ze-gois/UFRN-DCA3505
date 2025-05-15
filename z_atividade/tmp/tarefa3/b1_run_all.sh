#!/bin/bash
#
# b1_run_all.sh - Script para execução sequencial dos programas
# 
# Este script executa os programas de demonstração de processos em sequência
# e exibe informações básicas sobre os processos criados.
# 
# Objetivos didáticos:
# 1. Demonstrar como os PIDs mudam em múltiplas execuções do mesmo programa
# 2. Observar o comportamento do fork() na criação de processos filho
# 3. Visualizar a hierarquia de processos usando ps
#
# Run all programs and collect data
echo "=== Running Process ID Program ==="
echo "First execution:"
./process_id
echo
echo "Second execution:"
./process_id
echo
echo "Third execution:"
./process_id
echo

echo "=== Running Fork Process Program ==="
./fork_process
echo

echo "=== Process Tree Information ==="
echo "Current process tree for the terminal:"
ps f
echo

echo "=== For detailed process observation ==="
echo "To observe processes in real-time with detailed metrics, run:"
echo "  ./b2_observe_processes.sh"
echo
echo "This will run the processes in the background and save detailed logs."
echo

echo "All done! Review the output above for your assignment."
echo "Remember to note the PIDs and PPIDs for your report."