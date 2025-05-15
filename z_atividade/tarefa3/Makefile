all:
	gcc -o fp ./fork_pid.c

	echo -n "Repetitive: "; \
	for f2 in $$(seq 1 30); do \
	    ./fp > "./log/$$f2.log"; echo "$$f2.log, "; \
	done
