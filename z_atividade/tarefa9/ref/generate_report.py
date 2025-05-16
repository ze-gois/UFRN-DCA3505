#!/usr/bin/env python3
"""
Script to generate a LaTeX report from scheduling experiment data
"""

import os
import re
import glob
import datetime
import statistics
from collections import defaultdict

def get_log_files(experiment_name=None):
    """Get log files for a specific experiment or all experiments"""
    if experiment_name:
        return sorted(glob.glob(f"./log/ps-{experiment_name}-*.log"))
    else:
        return sorted(glob.glob("./log/ps-*.log"))

def parse_log_file(log_file):
    """Parse a log file and extract process information"""
    processes = []
    
    with open(log_file, 'r') as f:
        lines = f.readlines()
        
    # Find the header line that includes the column names
    header_line = None
    for i, line in enumerate(lines):
        if "PID" in line and "CPU" in line and "STAT" in line:
            header_line = i
            break
            
    if header_line is None:
        return processes
        
    # Extract column positions
    header = lines[header_line]
    pid_pos = header.find("PID")
    pri_pos = header.find("PRI")
    ni_pos = header.find("NI")
    stat_pos = header.find("STAT")
    cpu_pos = header.find("%CPU")
    time_pos = header.find("TIME+")
    cmd_pos = header.find("CMD")
    
    # Parse process data
    for i in range(header_line + 1, len(lines)):
        line = lines[i].strip()
        if not line:
            continue
        
        try:
            pid = int(line[pid_pos:pri_pos].strip())
            pri = int(line[pri_pos:ni_pos].strip())
            ni = int(line[ni_pos:stat_pos].strip())
            stat = line[stat_pos:cpu_pos].strip()
            cpu = float(line[cpu_pos:time_pos].strip())
            time = line[time_pos:cmd_pos].strip()
            cmd = line[cmd_pos:].strip()
            
            processes.append({
                'pid': pid,
                'pri': pri,
                'ni': ni,
                'stat': stat,
                'cpu': cpu,
                'time': time,
                'cmd': cmd
            })
        except (ValueError, IndexError) as e:
            # Skip problematic lines
            continue
    
    return processes

def analyze_experiment(experiment_name):
    """Analyze data for a specific experiment"""
    log_files = get_log_files(experiment_name)
    if not log_files:
        return None
    
    all_processes = []
    for log_file in log_files:
        processes = parse_log_file(log_file)
        all_processes.append(processes)
    
    # Group processes by PID to track over time
    processes_by_pid = defaultdict(list)
    for processes in all_processes:
        for proc in processes:
            processes_by_pid[proc['pid']].append(proc)
    
    # Calculate average CPU usage per process
    avg_cpu_by_pid = {}
    for pid, procs in processes_by_pid.items():
        if procs:  # Make sure we have data
            cpu_values = [p['cpu'] for p in procs]
            avg_cpu = statistics.mean(cpu_values) if cpu_values else 0
            avg_cpu_by_pid[pid] = avg_cpu
    
    # Get most common state for each process
    state_by_pid = {}
    for pid, procs in processes_by_pid.items():
        if procs:
            states = [p['stat'] for p in procs]
            most_common_state = max(set(states), key=states.count)
            state_by_pid[pid] = most_common_state
    
    # Separate busy and monitoring processes
    busy_processes = {}
    blocking_processes = {}
    monitoring_processes = {}
    
    for pid, procs in processes_by_pid.items():
        if not procs:
            continue
            
        last_proc = procs[-1]  # Use the last sample for classification
        if "busy-proc" in last_proc['cmd']:
            busy_processes[pid] = {
                'cmd': last_proc['cmd'],
                'avg_cpu': avg_cpu_by_pid.get(pid, 0),
                'state': state_by_pid.get(pid, ""),
                'ni': last_proc['ni']
            }
        elif "blocking-proc" in last_proc['cmd']:
            blocking_processes[pid] = {
                'cmd': last_proc['cmd'],
                'avg_cpu': avg_cpu_by_pid.get(pid, 0),
                'state': state_by_pid.get(pid, ""),
                'ni': last_proc['ni']
            }
        elif "ps-monitor" in last_proc['cmd'] or "ps" in last_proc['cmd']:
            monitoring_processes[pid] = {
                'cmd': last_proc['cmd'],
                'avg_cpu': avg_cpu_by_pid.get(pid, 0),
                'state': state_by_pid.get(pid, ""),
                'ni': last_proc['ni']
            }
    
    # Calculate statistics
    busy_cpu_values = [p['avg_cpu'] for p in busy_processes.values()]
    
    return {
        'experiment_name': experiment_name,
        'num_samples': len(all_processes),
        'busy_processes': busy_processes,
        'blocking_processes': blocking_processes,
        'monitoring_processes': monitoring_processes,
        'avg_cpu_overall': statistics.mean(busy_cpu_values) if busy_cpu_values else 0,
        'max_cpu': max(busy_cpu_values) if busy_cpu_values else 0,
        'min_cpu': min(busy_cpu_values) if busy_cpu_values else 0,
        'std_dev_cpu': statistics.stdev(busy_cpu_values) if len(busy_cpu_values) > 1 else 0,
        'cpu_fairness': (max(busy_cpu_values) - min(busy_cpu_values)) if busy_cpu_values else 0,
    }

def generate_latex_report():
    """Generate LaTeX report from experiment data"""
    # Find all experiment names
    experiment_patterns = [
        "Part1-N_Processes",
        "Part2-N_Plus_1_Processes", 
        "Part3-Priority_Effect",
        "Part4-Blocked_Process"
    ]
    
    # Analyze each experiment
    results = {}
    for exp_name in experiment_patterns:
        results[exp_name] = analyze_experiment(exp_name)
    
    # Start generating LaTeX document
    latex_content = []
    
    # Document preamble
    latex_content.append(r"""\documentclass{article}
\usepackage[utf8]{inputenc}
\usepackage{graphicx}
\usepackage{booktabs}
\usepackage{amsmath}
\usepackage{geometry}
\usepackage{float}
\usepackage{color}
\usepackage{hyperref}

\geometry{a4paper, margin=1in}

\title{Análise do Escalonador do Linux}
\author{Experimento de Escalonamento de Processos}
\date{\today}

\begin{document}

\maketitle

\section{Introdução}

Este relatório apresenta uma análise do comportamento do escalonador do Linux (CFS - Completely Fair Scheduler) em diferentes cenários. 
O experimento foi dividido em quatro partes:

\begin{enumerate}
    \item Distribuição com N Processos
    \item Sobrecarga com N+1 Processos
    \item Efeito da Prioridade
    \item Processo Bloqueado por Entrada
\end{enumerate}

\section{Metodologia}

Para cada experimento, criamos processos que executam laços infinitos consumindo CPU e monitoramos seu comportamento 
usando comandos como \texttt{ps}. As métricas coletadas incluem uso de CPU, estado dos processos, tempo de execução e prioridade.

""")
    
    # Add experiment results
    latex_content.append(r"\section{Resultados}")
    
    # Part 1 - N Processes
    exp_name = "Part1-N_Processes"
    if exp_name in results and results[exp_name]:
        latex_content.append(r"\subsection{Experimento 1: Distribuição com N Processos}")
        data = results[exp_name]
        
        latex_content.append(r"""
Neste experimento, foram criados N processos (onde N é o número de núcleos do processador) 
executando laços infinitos para consumir CPU continuamente.

\subsubsection{Observações}

""")
        
        # CPU usage data
        latex_content.append(f"Número de processos ocupados: {len(data['busy_processes'])}\n")
        latex_content.append(f"Uso médio de CPU por processo: {data['avg_cpu_overall']:.2f}\\%\n")
        latex_content.append(f"Desvio padrão do uso de CPU: {data['std_dev_cpu']:.2f}\\%\n")
        latex_content.append(f"Diferença entre máximo e mínimo uso de CPU: {data['cpu_fairness']:.2f}\\%\n\n")
        
        # Process table
        latex_content.append(r"""
\begin{table}[H]
\centering
\caption{Estatísticas dos Processos no Experimento 1}
\begin{tabular}{lrrrc}
\toprule
\textbf{Processo} & \textbf{PID} & \textbf{\%CPU} & \textbf{Nice} & \textbf{Estado} \\
\midrule
""")
        
        # Add process data rows
        for pid, proc in data['busy_processes'].items():
            latex_content.append(f"{proc['cmd']} & {pid} & {proc['avg_cpu']:.2f}\\% & {proc['ni']} & {proc['state']} \\\\\n")
        
        latex_content.append(r"""\bottomrule
\end{tabular}
\end{table}

\subsubsection{Análise}
Com N processos em N núcleos, o escalonador CFS do Linux tenta distribuir o tempo de CPU igualmente entre todos os processos.
Como cada processo está consumindo 100\% de um núcleo, esperamos ver aproximadamente distribuição igual de CPU.
""")
    
    # Part 2 - N+1 Processes
    exp_name = "Part2-N_Plus_1_Processes"
    if exp_name in results and results[exp_name]:
        latex_content.append(r"\subsection{Experimento 2: Sobrecarga com N+1 Processos}")
        data = results[exp_name]
        
        latex_content.append(r"""
Neste experimento, foram criados N+1 processos (mais do que o número de núcleos) 
para observar como o Linux gerencia a sobrecarga de processos.

\subsubsection{Observações}

""")
        
        # CPU usage data
        latex_content.append(f"Número de processos ocupados: {len(data['busy_processes'])}\n")
        latex_content.append(f"Uso médio de CPU por processo: {data['avg_cpu_overall']:.2f}\\%\n")
        latex_content.append(f"Desvio padrão do uso de CPU: {data['std_dev_cpu']:.2f}\\%\n")
        latex_content.append(f"Diferença entre máximo e mínimo uso de CPU: {data['cpu_fairness']:.2f}\\%\n\n")
        
        # Process table
        latex_content.append(r"""
\begin{table}[H]
\centering
\caption{Estatísticas dos Processos no Experimento 2}
\begin{tabular}{lrrrc}
\toprule
\textbf{Processo} & \textbf{PID} & \textbf{\%CPU} & \textbf{Nice} & \textbf{Estado} \\
\midrule
""")
        
        # Add process data rows
        for pid, proc in data['busy_processes'].items():
            latex_content.append(f"{proc['cmd']} & {pid} & {proc['avg_cpu']:.2f}\\% & {proc['ni']} & {proc['state']} \\\\\n")
        
        latex_content.append(r"""\bottomrule
\end{tabular}
\end{table}

\subsubsection{Análise}
Com N+1 processos competindo por N núcleos, o escalonador CFS precisa compartilhar os recursos de CPU disponíveis.
Idealmente, cada processo deveria receber aproximadamente a mesma fatia de tempo de CPU, resultando em uma distribuição
proporcional a N/(N+1) do tempo total disponível para cada processo.
""")
    
    # Part 3 - Priority Effect
    exp_name = "Part3-Priority_Effect"
    if exp_name in results and results[exp_name]:
        latex_content.append(r"\subsection{Experimento 3: Efeito da Prioridade}")
        data = results[exp_name]
        
        latex_content.append(r"""
Neste experimento, foram criados N processos, mas a prioridade de um deles foi aumentada 
usando \texttt{renice -n -10 -p <PID>} para observar o impacto da prioridade no escalonamento.

\subsubsection{Observações}

""")
        
        # CPU usage data
        latex_content.append(f"Número de processos ocupados: {len(data['busy_processes'])}\n")
        latex_content.append(f"Uso médio de CPU por processo: {data['avg_cpu_overall']:.2f}\\%\n")
        latex_content.append(f"Desvio padrão do uso de CPU: {data['std_dev_cpu']:.2f}\\%\n")
        latex_content.append(f"Diferença entre máximo e mínimo uso de CPU: {data['cpu_fairness']:.2f}\\%\n\n")
        
        # Process table
        latex_content.append(r"""
\begin{table}[H]
\centering
\caption{Estatísticas dos Processos no Experimento 3}
\begin{tabular}{lrrrc}
\toprule
\textbf{Processo} & \textbf{PID} & \textbf{\%CPU} & \textbf{Nice} & \textbf{Estado} \\
\midrule
""")
        
        # Add process data rows
        for pid, proc in data['busy_processes'].items():
            latex_content.append(f"{proc['cmd']} & {pid} & {proc['avg_cpu']:.2f}\\% & {proc['ni']} & {proc['state']} \\\\\n")
        
        latex_content.append(r"""\bottomrule
\end{tabular}
\end{table}

\subsubsection{Análise}
No CFS, processos com maior prioridade (nice value mais baixo) devem receber mais tempo de CPU.
O escalonador ajusta o tempo virtual de execução (vruntime) para processos com prioridade mais alta,
permitindo que eles sejam executados por mais tempo antes de serem preemptados.
""")
    
    # Part 4 - Blocked Process
    exp_name = "Part4-Blocked_Process"
    if exp_name in results and results[exp_name]:
        latex_content.append(r"\subsection{Experimento 4: Processo Bloqueado por Entrada}")
        data = results[exp_name]
        
        latex_content.append(r"""
Neste experimento, foram criados N processos intensivos de CPU mais um processo adicional 
que aguarda entrada do usuário (bloqueado por I/O).

\subsubsection{Observações}

""")
        
        # CPU usage data
        latex_content.append(f"Número de processos ocupados: {len(data['busy_processes'])}\n")
        if data['blocking_processes']:
            latex_content.append("Processo bloqueado encontrado no monitoramento.\n")
        else:
            latex_content.append("Nenhum processo bloqueado encontrado no monitoramento.\n")
        
        latex_content.append(f"Uso médio de CPU por processo intensivo: {data['avg_cpu_overall']:.2f}\\%\n")
        latex_content.append(f"Desvio padrão do uso de CPU: {data['std_dev_cpu']:.2f}\\%\n\n")
        
        # Process table for busy processes
        latex_content.append(r"""
\begin{table}[H]
\centering
\caption{Estatísticas dos Processos Intensivos no Experimento 4}
\begin{tabular}{lrrrc}
\toprule
\textbf{Processo} & \textbf{PID} & \textbf{\%CPU} & \textbf{Nice} & \textbf{Estado} \\
\midrule
""")
        
        # Add busy process data rows
        for pid, proc in data['busy_processes'].items():
            latex_content.append(f"{proc['cmd']} & {pid} & {proc['avg_cpu']:.2f}\\% & {proc['ni']} & {proc['state']} \\\\\n")
        
        latex_content.append(r"""\bottomrule
\end{tabular}
\end{table}
""")
        
        # Process table for blocking process
        if data['blocking_processes']:
            latex_content.append(r"""
\begin{table}[H]
\centering
\caption{Estatísticas do Processo Bloqueado no Experimento 4}
\begin{tabular}{lrrrc}
\toprule
\textbf{Processo} & \textbf{PID} & \textbf{\%CPU} & \textbf{Nice} & \textbf{Estado} \\
\midrule
""")
            
            # Add blocking process data rows
            for pid, proc in data['blocking_processes'].items():
                latex_content.append(f"{proc['cmd']} & {pid} & {proc['avg_cpu']:.2f}\\% & {proc['ni']} & {proc['state']} \\\\\n")
            
            latex_content.append(r"""\bottomrule
\end{tabular}
\end{table}
""")
        
        latex_content.append(r"""
\subsubsection{Análise}
Os processos bloqueados por I/O (como esperando entrada do usuário) geralmente
entram em estado de sono (estado S) e não consomem CPU enquanto bloqueados.
Quando recebem entrada, são acordados pelo escalonador e competem por tempo de CPU.
""")
    
    # Comparison section
    latex_content.append(r"""
\section{Análise Comparativa}

\subsection{Comparação do CFS com Outros Algoritmos de Escalonamento}

\begin{table}[H]
\centering
\caption{Comparação de Algoritmos de Escalonamento}
\begin{tabular}{lp{12cm}}
\toprule
\textbf{Algoritmo} & \textbf{Características} \\
\midrule
CFS (Linux) & O Completely Fair Scheduler utiliza uma árvore rubro-negra para gerenciar processos e garantir distribuição justa de recursos. Cada processo recebe uma quantidade de tempo proporcional ao seu peso (prioridade). O CFS minimiza a latência de espera, favorecendo processos interativos. \\
\midrule
FIFO & First-In-First-Out é um algoritmo não-preemptivo que executa os processos na ordem de chegada até a conclusão. Favorece processos que chegam primeiro, independentemente do tamanho e de requisitos de CPU. \\
\midrule
Round Robin & Algoritmo preemptivo que aloca um quantum de tempo para cada processo em uma fila circular. Garante que todos os processos tenham chance de execução, mas pode ser ineficiente com muita alternância de contexto. \\
\midrule
SJF & Shortest Job First prioriza processos com menor tempo de execução estimado. Minimiza o tempo médio de espera, mas pode causar inanição (starvation) de processos longos. \\
\bottomrule
\end{tabular}
\end{table}

\subsection{Vantagens do CFS}

O Completely Fair Scheduler (CFS) do Linux possui várias vantagens:

\begin{itemize}
    \item \textbf{Justiça}: Garante que todos os processos recebam uma parte justa do tempo de CPU.
    \item \textbf{Escalabilidade}: Utiliza estruturas de dados eficientes (árvore rubro-negra) para suportar muitos processos.
    \item \textbf{Responsividade}: Favorece processos interativos através do modelo de tempo virtual.
    \item \textbf{Priorização flexível}: Permite ajuste de prioridades através de valores nice.
\end{itemize}

\section{Conclusão}

Com base nos experimentos realizados, podemos concluir que o escalonador CFS do Linux:

\begin{enumerate}
    \item Distribui eficientemente o tempo de CPU entre N processos em N núcleos;
    \item Mantém uma distribuição justa mesmo quando há mais processos que núcleos disponíveis;
    \item Respeita as prioridades dos processos, dando mais tempo aos processos com maior prioridade;
    \item Gerencia adequadamente processos bloqueados por I/O, permitindo que eles consumam CPU apenas quando necessário.
\end{enumerate}

O CFS implementa um modelo de justiça que difere significativamente de algoritmos mais simples como FIFO ou Round Robin.
Ao utilizar o conceito de tempo virtual de execução (vruntime), o CFS consegue balancear as necessidades de processos 
com diferentes prioridades e características de execução.

\end{document}
""")
    
    # Combine all latex content
    return "".join(latex_content)

if __name__ == "__main__":
    # Make sure log directory exists
    if not os.path.exists("./log"):
        print("Log directory not found. Make sure you've run the experiments.")
        os.makedirs("./log", exist_ok=True)
    
    # Generate LaTeX report
    report_content = generate_latex_report()
    
    # Save to file
    with open("report.tex", "w") as f:
        f.write(report_content)
    
    print("LaTeX report generated: report.tex")
    print("To compile: pdflatex report.tex")