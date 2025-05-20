all: run

assemble:
	as -o _start.o _start.s

link: assemble
	ld -o _start _start.o

run: link
	./_start

clean:
	@rm _start.o
	@rm _start
