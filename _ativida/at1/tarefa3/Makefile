CC = gcc
CFLAGS = -Wall -Wextra -pedantic
TARGETS = process_id fork_process

all: $(TARGETS)
	gcc -o o_fifo a2_fork_process_fifo.c
	gcc -o o_flock a2_fork_process_flock.c
	gcc -o o_msgq a2_fork_process_msgq.c
	gcc -o o_pipes a2_fork_process_pipes.c
	gcc -o o_sem a2_fork_process_sem.c
	gcc -o o_shm a2_fork_process_shm.c
	gcc -o o_signal a2_fork_process_signal.c
	gcc -o o_socket a2_fork_process_socket.c

	./o_fifo > o_fifo.log &
	./o_flock > o_flock.log &
	./o_msgq > o_msgq.log &
	./o_pipes > o_pipes.log &
	./o_sem > o_sem.log &
	./o_shm > o_shm.log &
	./o_signal > o_signal.log &
	./o_socket > o_socket.log &

process_id: a1_process_id.c
	$(CC) $(CFLAGS) -o $@ $<

fork_process: a2_fork_process.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
