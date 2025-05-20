all:
	@echo "make run"

run: data_race
	./data_race

compile-verbose: ./e01_data_race.c
	gcc -o data_race ./e01_data_race.c -static

compile-silent: ./e01_data_race_silent.c
	gcc -o data_race_silent ./e01_data_race_silent.c -static

diassemble-silent: data_race_silent



strace:
	strace ./data_race_silent
