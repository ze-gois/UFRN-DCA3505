# Compilando o Relatório em LaTeX

Este documento explica como compilar a versão LaTeX do relatório para gerar um PDF profissional.

## Pré-requisitos

Para compilar o documento LaTeX, você precisará das seguintes dependências:

```bash
sudo apt-get install texlive-latex-base texlive-fonts-recommended texlive-extra-utils texlive-latex-extra
```

Ou, para uma instalação mais completa:

```bash
sudo apt-get install texlive-full
```

## Compilação Automática

O Makefile inclui um comando para compilar o relatório LaTeX automaticamente:

```bash
make pdf
```

Este comando executa o script `compile_latex.sh`, que:
1. Verifica se pdflatex está instalado
2. Compila o documento em duas passagens para resolver referências
3. Remove arquivos temporários
4. Verifica se o PDF foi gerado com sucesso

## Compilação Manual

Se preferir compilar manualmente, use os seguintes comandos:

```bash
pdflatex relatorio_tarefa3.tex
pdflatex relatorio_tarefa3.tex  # Segunda passagem para resolver referências
```

## Estrutura do Documento LaTeX

O arquivo `relatorio_tarefa3.tex` contém:
- Configuração de pacotes e formatação
- Formatação profissional de blocos de código
- Tabela com os resultados do experimento de oscilação de PIDs
- Todas as respostas para as perguntas da tarefa
- Conclusões baseadas nas observações experimentais

## Geração do Relatório Completo

Para executar todos os experimentos e gerar o relatório PDF final, use:

```bash
make report
```

Este comando:
1. Compila todos os programas (`pid_info`, `fork_example`, `infinite_loop`)
2. Executa o script de análise de processos
3. Executa o experimento de oscilação de PIDs
4. Gera o PDF a partir do arquivo LaTeX

## Limpeza

Para limpar todos os arquivos gerados:

```bash
make clean
```

Este comando remove os executáveis compilados, o PDF final e os arquivos temporários de LaTeX.