#!/bin/bash

echo "Compilando relatório LaTeX para PDF..."

# Verifica se pdflatex está instalado
if ! command -v pdflatex &> /dev/null; then
    echo "ERRO: Este script requer pdflatex para gerar PDF."
    echo "Por favor, instale com:"
    echo "  sudo apt-get install texlive-latex-base texlive-fonts-recommended texlive-extra-utils texlive-latex-extra"
    exit 1
fi

# Verifica se o arquivo .tex existe
if [ ! -f relatorio_tarefa3.tex ]; then
    echo "ERRO: Arquivo relatorio_tarefa3.tex não encontrado."
    exit 1
fi

# Compila o documento (2 vezes para garantir que referências sejam resolvidas)
echo "Executando pdflatex (primeira passagem)..."
pdflatex -interaction=nonstopmode relatorio_tarefa3.tex > /dev/null

echo "Executando pdflatex (segunda passagem para finalizar referências)..."
pdflatex -interaction=nonstopmode relatorio_tarefa3.tex > /dev/null

# Limpa arquivos temporários
echo "Limpando arquivos temporários..."
rm -f relatorio_tarefa3.aux relatorio_tarefa3.log relatorio_tarefa3.out relatorio_tarefa3.toc

# Verifica se o PDF foi gerado com sucesso
if [ -f relatorio_tarefa3.pdf ]; then
    echo "PDF gerado com sucesso: relatorio_tarefa3.pdf"
    ls -lh relatorio_tarefa3.pdf
else
    echo "ERRO: Falha ao gerar o PDF."
    exit 1
fi