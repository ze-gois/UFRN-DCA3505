all: run sequence wait

syntax: ./c00_syntax/fork_syntax.c
	gcc -o ./c00_syntax/fork_syntax ./c00_syntax/fork_syntax.c
	./c00_syntax/fork_syntax

sequence: ./c01_sequence/fork_sequence.c
	gcc -o ./c01_sequence/fork_sequence ./c01_sequence/fork_sequence.c
	@for f1 in $$(seq 1 10); do \
	   echo -n "Sequence: "; \
	   for f2 in $$(seq 1 333); do \
	      ./c01_sequence/fork_sequence; echo; \
	   done | grep '^12$$' -o | wc -l; \
	done

wait: ./c02_wait_sequence/fork_wait_sequence.c
	gcc -o ./c02_wait_sequence/fork_wait_sequence ./c02_wait_sequence/fork_wait_sequence.c
	@for f1 in $$(seq 1 3); do \
	   echo -n "Wait sequence: "; \
	   for f2 in $$(seq 1 333); do \
	      ./c02_wait_sequence/fork_wait_sequence; echo; \
	   done | grep '^12$$' -o | wc -l; \
	done

clean:
	rm -f ./c00_syntax/fork_syntax
	rm -f ./c01_sequence/fork_sequence
	rm -f ./c02_wait_sequence/fork_wait_sequence
