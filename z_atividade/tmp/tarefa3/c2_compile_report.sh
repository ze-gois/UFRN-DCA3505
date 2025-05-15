#!/bin/bash
#
# c2_compile_report.sh - Script para compilação do relatório LaTeX
# 
# Este script compila o arquivo LaTeX do relatório para gerar o PDF final.
# 
# Objetivos didáticos:
# 1. Automatizar o processo de compilação do relatório
# 2. Demonstrar o uso de scripts para tarefas repetitivas
# 3. Facilitar a geração do documento final para entrega
#

# Check if pdflatex is installed
if ! command -v pdflatex &> /dev/null; then
    echo "Error: pdflatex is not installed. Please install TeX Live or another LaTeX distribution."
    exit 1
fi

# Compile the report
pdflatex c1_relatorio.tex
pdflatex c1_relatorio.tex  # Run twice to resolve references

# Clean up auxiliary files
rm -f c1_relatorio.aux c1_relatorio.log c1_relatorio.out c1_relatorio.toc

echo "Report compilation complete. Output file: c1_relatorio.pdf"