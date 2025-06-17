all: run report

report:
	lualatex report.tex

run: compile
	./main > main.log
	objdump -D ./main > ./main.dump
	tail main.log -n 1

	./main_o > main_o.log
	objdump -D ./main_o > ./main_o.dump
	tail main_o.log -n 1

	./main_atomic > main_atomic.log
	objdump -D ./main_atomic > ./main_atomic.dump
	tail main_atomic.log -n 1

	./main_mutex > main_mutex.log
	objdump -D ./main_mutex > ./main_mutex.dump
	tail main_mutex.log -n 1

compile:
	gcc -o main ./main.c -pthread
	gcc -o main_o ./main.c -pthread -O3
	gcc -o main_atomic ./main_atomic.c -pthread
	gcc -o main_mutex ./main_mutex.c -pthread
