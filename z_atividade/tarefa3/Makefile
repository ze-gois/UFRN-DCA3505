all:
	gcc -o fp ./fork_pid.c
	rm -rf ./log
	mkdir ./log
	echo -n "Repetitive: "; \
	for f2 in $$(seq 1 300); do \
	    ./fp > "./log/$$f2.log"; echo "$$f2.log, "; \
	done
