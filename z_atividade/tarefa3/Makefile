all: run report

run:
	gcc -o fp ./fork_pid.c
	rm -rf ./log
	mkdir ./log
	echo -n "Repetitive: "; \
	for f2 in $$(seq 1 3); do \
	    ./fp > "./log/$$f2.log"; echo -n "$$f2.log, "; \
	done
	./analysis/b00_first.py

report: run
	pdflatex -shell-escape report_simple.tex
	pdflatex -shell-escape report_simple.tex

clean:
	rm -f fp
	rm -f *.o
	rm -f *.aux *.log *.out
	rm -rf ./_minted*
	rm -f *.synctex.gz
	rm -f _*.config.minted
	rm -f *.toc *.lof *.lot
	rm -rf log/0.df

distclean: clean
	rm -rf ./log
	rm -rf ./figures
	rm -f *.pdf
