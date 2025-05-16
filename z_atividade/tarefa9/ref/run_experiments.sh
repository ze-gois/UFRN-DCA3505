#!/bin/bash

# Run Experiments and Generate Report
# This script runs the scheduling experiments and generates a report

echo "===== Linux Scheduler Experiment Runner ====="
echo ""

# Make sure we're in the right directory
cd "$(dirname "$0")"

# Check if 'log' directory exists, create if not
mkdir -p log

# Clean up existing logs
echo "Cleaning up old log files..."
rm -f ./log/ps-*.log

# Compile the program
echo "Compiling the program..."
make clean
make

# Run the experiments
echo "Running scheduling experiments..."
echo "(This might take a minute or two)"
echo ""
echo "Note: For part 4, the process will wait for input."
echo "When prompted, type something and press Enter to continue."
echo ""

# Run the program
./spawn

# Check if experiments completed successfully
if [ $? -eq 0 ]; then
    echo "All experiments completed successfully!"
    
    # Generate the report
    echo "Generating LaTeX report..."
    python3 generate_report.py
    
    # Check if pdflatex is installed
    if command -v pdflatex &> /dev/null; then
        echo "Compiling PDF report..."
        pdflatex -interaction=nonstopmode report.tex
        pdflatex -interaction=nonstopmode report.tex  # Run twice for references
        
        echo ""
        echo "Report generated: report.pdf"
    else
        echo ""
        echo "Report generated: report.tex"
        echo "To create PDF, install LaTeX and run: pdflatex report.tex"
    fi
else
    echo "Error running experiments. Check the output above for details."
fi

echo ""
echo "===== Experiment complete ====="