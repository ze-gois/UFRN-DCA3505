all: main report

main: main.c
	gcc -o main main.c

run: main
	./main

report: report.tex
	lualatex report.tex

clean:
	rm -f main report.pdf *.aux *.log *.out
