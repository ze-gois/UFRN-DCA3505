# Makefile for Assembly Demonstration
# Handles compilation, tracing and report generation for minimal assembly programs

# Tools
AS = as
LD = ld
LATEX = pdflatex
STRACE = strace

# Files
SRC = minimal.s minimal_fixed.s
OBJ = minimal.o minimal_fixed.o
EXEC = minimal minimal_fixed

# Default target - runs everything
all: distclean demo report

# Compilation rules
%.o: %.s
	$(AS) -o $@ $<

minimal: minimal.o
	$(LD) -o $@ $<

minimal_fixed: minimal_fixed.o
	$(LD) -o $@ $<

# Demo execution
demo: $(EXEC)
	@echo "========= RUNNING MINIMAL PROGRAM (WILL CAUSE SEGFAULT) ========="
	-./minimal
	@echo "========= TRACING MINIMAL PROGRAM ========="
	-$(STRACE) ./minimal
	@echo ""
	@echo "========= RUNNING FIXED PROGRAM ========="
	./minimal_fixed
	@echo "Exit code: $$?"
	@echo "========= TRACING FIXED PROGRAM ========="
	$(STRACE) ./minimal_fixed
	@echo ""

# Generate report
report:
	@echo "========= GENERATING REPORT ========="
	$(LATEX) relatorio.tex > relatorio.1.log
	$(LATEX) relatorio.tex  > relatorio.2.log
	@echo "Report generated: relatorio.pdf"

# Clean up
clean:
	rm -f $(OBJ) $(EXEC) *.aux *.log *.out

# Clean up including PDF
distclean: clean
	rm -f *.pdf

.PHONY: all demo report clean distclean
