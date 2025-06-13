all: spawn report

spawn:
	rm -rf ./log
	mkdir ./log
	gcc -o spawn ./spawn.c
	./spawn

report:
	python3 analysis.py
	pdflatex report_pt.tex
	pdflatex report_pt.tex
	@echo "Report generated: report_pt.pdf"

clean:
	rm -f *.aux *.log *.out *.toc *.lof *.lot *.fls *.fdb_latexmk *.synctex.gz
	rm -f spawn

distclean: clean
	rm -rf ./log
	rm -rf ./figures
	rm -rf ./latex
	rm -f report_pt.pdf
	rm -f *.csv *.png
