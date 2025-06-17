all: run report

report:
	lualatex report.tex

run: compile
	./main

compile:
	gcc -o main ./main.c -pthread
