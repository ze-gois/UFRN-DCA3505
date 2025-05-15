# Makefile for System Call Demonstration
# Handles compilation, execution, tracing and report generation for C and Assembly programs

# Tools
CC = gcc
AS = as
LD = ld
LATEX = pdflatex
STRACE = strace

# Files
ASM_SRC = write_asm.s
C_SRC = write_c.c
ASM_OBJ = write_asm.o
C_OBJ = write_c.o
ASM_EXEC = write_asm
C_EXEC = write_c
EXECS = $(ASM_EXEC) $(C_EXEC)

# Default target - runs everything
all: distclean compile demo report

# Compilation rules
compile: $(EXECS)

%.o: %.s
	$(AS) -o $@ $<

$(ASM_EXEC): $(ASM_OBJ)
	$(LD) -o $@ $<

$(C_EXEC): $(C_SRC)
	$(CC) -static -o $@ $<

# Demo execution
demo: $(EXECS)
	@echo "========= RUNNING C PROGRAM ========="
	./$(C_EXEC)
	@echo "Exit code: $$?"
	@echo "========= TRACING C PROGRAM ========="
	$(STRACE) ./$(C_EXEC)
	@echo ""
	@echo "========= RUNNING ASSEMBLY PROGRAM ========="
	./$(ASM_EXEC)
	@echo "Exit code: $$?"
	@echo "========= TRACING ASSEMBLY PROGRAM ========="
	$(STRACE) ./$(ASM_EXEC)
	@echo ""

# Generate report
report:
	@echo "========= GENERATING REPORT ========="
	$(LATEX) relatorio.tex > relatorio.1.log
	$(LATEX) relatorio.tex > relatorio.2.log
	@echo "Report generated: relatorio.pdf"

# Clean up
clean:
	rm -f $(ASM_OBJ) $(C_OBJ) $(EXECS) *.aux *.log *.out

# Clean up including PDF
distclean: clean
	rm -f *.pdf

.PHONY: all compile demo report clean distclean