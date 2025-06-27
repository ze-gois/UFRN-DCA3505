# Makefile for Producer-Consumer examples
CC = gcc
CFLAGS = -Wall -pthread

TARGETS = original mutex_solution semaphore_solution condition_var_solution

all: $(TARGETS)

original: original.c
	$(CC) $(CFLAGS) -o original original.c

mutex_solution: mutex_solution.c
	$(CC) $(CFLAGS) -o mutex_solution mutex_solution.c

semaphore_solution: semaphore_solution.c
	$(CC) $(CFLAGS) -o semaphore_solution semaphore_solution.c

condition_var_solution: condition_var_solution.c
	$(CC) $(CFLAGS) -o condition_var_solution condition_var_solution.c

clean:
	rm -f $(TARGETS)
