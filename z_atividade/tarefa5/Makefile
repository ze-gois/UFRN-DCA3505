all: run report

report:
	lualatex report.tex

run: compile
	./main > main.log
	tail main.log -n 1
	./main_o > main_o.log
	tail main_o.log -n 1

compile:
	gcc -o main ./main.c -pthread
	gcc -o main_o ./main.c -pthread -O3
