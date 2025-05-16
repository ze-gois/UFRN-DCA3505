# Linux Scheduler Analysis Experiment

This project demonstrates and analyzes the behavior of the Linux Completely Fair Scheduler (CFS) under different workloads and conditions.

## Experiment Overview

The experiment is divided into four parts:

1. **Part 1 - Distribution with N Processes**
   - Creates N CPU-intensive processes (where N is the number of CPU cores)
   - Observes how the scheduler distributes CPU time
   
2. **Part 2 - Overload with N+1 Processes**
   - Creates N+1 CPU-intensive processes
   - Tests how the scheduler behaves with more processes than CPU cores
   
3. **Part 3 - Priority Effect**
   - Creates N processes with one having higher priority via `renice`
   - Shows how priority affects CPU allocation
   
4. **Part 4 - Process Blocked by Input**
   - Creates N CPU-intensive processes + 1 process waiting for input
   - Demonstrates the scheduler's behavior with I/O blocked processes

## Requirements

- Linux operating system
- GCC compiler
- Python 3 (for report generation)
- LaTeX (optional, for PDF report compilation)
- Standard command-line utilities: `ps`, `nproc`, `renice`, `kill`

## Usage

### Option 1: Run Everything Automatically

```bash
./run_experiments.sh
```

This script will:
1. Clean up any old logs
2. Compile the program
3. Run all experiments
4. Generate a LaTeX report
5. Compile the PDF if LaTeX is installed

### Option 2: Manual Execution

```bash
# Build the program
make

# Run the experiments
./spawn

# Generate LaTeX report
python3 generate_report.py

# Create PDF report (requires LaTeX)
pdflatex report.tex
```

## Understanding the Output

During execution, the program will:
- Log process data to files in the `log/` directory
- Show real-time CPU and process state information on the console
- For Part 4, you'll need to provide input when prompted

## Report

After running the experiments, a comprehensive report will be generated:
- `report.tex`: LaTeX source file
- `report.pdf`: Compiled PDF (if LaTeX is installed)

The report includes:
- Analysis of CPU distribution
- Effect of process overload
- Impact of priority changes
- Behavior with I/O blocked processes
- Comparison with other scheduling algorithms (FIFO, Round Robin, SJF)

## Project Structure

- `spawn.c`: Main experiment implementation
- `generate_report.py`: Python script to analyze logs and create report
- `run_experiments.sh`: Helper script to run everything
- `Makefile`: Build configuration
- `log/`: Directory containing experiment logs
- `report.tex`/`report.pdf`: Generated analysis

## Troubleshooting

- If processes don't terminate properly, use `killall spawn` to clean up
- For Part 4, make sure to provide input when prompted
- Check the `log/` directory for detailed process data