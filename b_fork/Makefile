all: run sequence wait

syntax: ./b00_syntax/fork_syntax.c
	gcc -o ./b00_syntax/fork_syntax ./b00_syntax/fork_syntax.c
	./b00_syntax/fork_syntax

sequence: ./b01_sequence/fork_sequence.c
	gcc -o ./b01_sequence/fork_sequence ./b01_sequence/fork_sequence.c
	@for f1 in $$(seq 1 10); do \
	   echo -n "Sequence: "; \
	   for f2 in $$(seq 1 333); do \
	      ./b01_sequence/fork_sequence; echo; \
	   done | grep '^12$$' -o | wc -l; \
	done

wait: ./b02_wait_sequence/fork_wait_sequence.c
	gcc -o ./b02_wait_sequence/fork_wait_sequence ./b02_wait_sequence/fork_wait_sequence.c
	@for f1 in $$(seq 1 3); do \
	   echo -n "Wait sequence: "; \
	   for f2 in $$(seq 1 333); do \
	      ./b02_wait_sequence/fork_wait_sequence; echo; \
	   done | grep '^12$$' -o | wc -l; \
	done

clean:
	rm -f ./b00_syntax/fork_syntax
	rm -f ./b01_sequence/fork_sequence
	rm -f ./b02_wait_sequence/fork_wait_sequence
