report:
	lualatex report.tex

compile:
	gcc -o main main.c -pthread
	gcc -o main_atomic main_atomic.c -pthread
	gcc -o main_atomicLanguage main_atomicLanguage.c -pthread -std=c11
	gcc -o main_barrier main_barrier.c -pthread
	gcc -o main_sync main_sync.c -pthread

run: compile
	./main
	./main_atomic
	./main_atomicLanguage
	./main_barrier
	./main_sync
