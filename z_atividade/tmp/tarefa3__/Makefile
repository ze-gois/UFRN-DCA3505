CC = gcc
CFLAGS = -Wall -Wextra -pedantic
TARGETS = process_id fork_process

all: $(TARGETS)

process_id: a1_process_id.c
	$(CC) $(CFLAGS) -o $@ $<

fork_process: a2_fork_process.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
