report:
	lualatex report.tex

compile:
	gcc -o main main.c -pthread -O2 -DOPTIMIZED=1
	gcc -o main_noopt main.c -pthread -O0 -DOPTIMIZED=0
	gcc -o main_atomic main_atomic.c -pthread
	gcc -o main_atomicLanguage main_atomicLanguage.c -pthread -std=c11
	gcc -o main_barrier main_barrier.c -pthread
	gcc -o main_sync main_sync.c -pthread

run: compile
	@echo "Executando versão padrão (com otimizações):"
	./main
	@echo ""
	@echo "Executando versão sem otimizações:"
	./main_noopt
	@echo ""
	@echo "Executando versão com operações atômicas:"
	./main_atomic
	@echo ""
	@echo "Executando versão com biblioteca atômica C11:"
	./main_atomicLanguage
	@echo ""
	@echo "Executando versão com barreiras de memória:"
	./main_barrier
	@echo ""
	@echo "Executando versão com mutex POSIX:"
	./main_sync
	@echo ""

clean:
	rm -f main main_noopt main_atomic main_atomicLanguage main_barrier main_sync
