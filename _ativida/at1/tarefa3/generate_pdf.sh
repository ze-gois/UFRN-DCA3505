#!/bin/bash

echo "Gerando relatório PDF..."

# Verifica se pandoc está instalado
if ! command -v pandoc &> /dev/null; then
    echo "ERRO: Este script requer pandoc para gerar PDF."
    echo "Por favor, instale com:"
    echo "  sudo apt-get install pandoc texlive-latex-base texlive-fonts-recommended texlive-extra-utils texlive-latex-extra"
    exit 1
fi

# Verifica se o README.md existe
if [ ! -f README.md ]; then
    echo "ERRO: Arquivo README.md não encontrado."
    exit 1
fi

# Gera o PDF
echo "Convertendo README.md para PDF..."
pandoc README.md \
    -o relatorio_tarefa3.pdf \
    --pdf-engine=xelatex \
    -V geometry:margin=1in \
    -V papersize=a4 \
    -V fontsize=11pt \
    -V mainfont="DejaVu Serif" \
    -V monofont="DejaVu Sans Mono" \
    -V lang=pt-BR \
    --toc

# Verifica se o PDF foi gerado com sucesso
if [ -f relatorio_tarefa3.pdf ]; then
    echo "PDF gerado com sucesso: relatorio_tarefa3.pdf"
    ls -lh relatorio_tarefa3.pdf
else
    echo "ERRO: Falha ao gerar o PDF."
    exit 1
fi