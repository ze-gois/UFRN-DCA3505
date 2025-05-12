CC = gcc
CFLAGS = -Wall -Wextra -pedantic

all: pid_info fork_example infinite_loop

pid_info: pid_info.c
	$(CC) $(CFLAGS) -o pid_info pid_info.c

fork_example: fork_example.c
	$(CC) $(CFLAGS) -o fork_example fork_example.c
	
infinite_loop: infinite_loop.c
	$(CC) $(CFLAGS) -o infinite_loop infinite_loop.c

clean:
	rm -f pid_info fork_example infinite_loop
	rm -f relatorio_tarefa3.pdf relatorio_tarefa3.aux relatorio_tarefa3.log relatorio_tarefa3.out relatorio_tarefa3.toc
	rm -rf logs/*
	killall -q infinite_loop 2>/dev/null || true

run_pid_info:
	./pid_info

run_fork:
	./fork_example

pid_oscillation:
	./pid_chaos_demo.sh

pid_oscillation_logged: all
	./pid_oscillation_experiment.sh

spawn_logged: all
	./spawn_processes_logged.sh

kill_logged: all
	./kill_random_logged.sh

pdf:
	./compile_latex.sh

report: all analysis pid_oscillation_logged pdf

pid_chaos: all
	./pid_chaos_demo.sh

logs_dir:
	mkdir -p logs

.PHONY: all clean run_pid_info run_fork analysis pdf report pid_chaos latex logs_dir pid_oscillation_logged spawn_logged kill_logged