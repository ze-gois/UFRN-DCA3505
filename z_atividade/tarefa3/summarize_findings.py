#!/usr/bin/python
import os
import re
import sys
from collections import defaultdict
from pathlib import Path

def extract_orphaned_processes():
    """Analyze log files and extract information about orphaned processes"""

    log_files = list(Path("log").glob("*.log"))
    if not log_files:
        print("Error: No log files found in the 'log' directory")
        sys.exit(1)

    print(f"Analyzing {len(log_files)} log files...")

    # Process data
    orphaned_processes = []
    all_processes = defaultdict(list)

    # Tracking for experiment statistics
    exp_stats = defaultdict(lambda: {'total_g': 0, 'orphaned_g': 0})

    for log_file in log_files:
        with open(log_file, 'r') as f:
            content = f.read()

            # Pattern to match log entries
            pattern = r"([A-Z]\d+)\t([A-Z])\t(\d+)\t(\d+)(?:\tsleep for (\d+) seconds|\twokeup|\tEnd\tof experiment|)?"
            matches = re.findall(pattern, content)

            for match in matches:
                exp_id, process_type, pid, ppid = match[0], match[1], int(match[2]), int(match[3])
                exp_letter = exp_id[0]

                # Track all grandchild processes
                if process_type == 'G':
                    exp_stats[exp_letter]['total_g'] += 1

                    # Add to orphaned list if PPID=1
                    if ppid == 1:
                        exp_stats[exp_letter]['orphaned_g'] += 1
                        orphaned_processes.append({
                            'exp_id': exp_id,
                            'exp_letter': exp_letter,
                            'pid': pid,
                            'ppid': ppid,
                            'log_file': log_file.name
                        })

                # Store process information for later analysis
                all_processes[pid].append({
                    'exp_id': exp_id,
                    'exp_letter': exp_letter,
                    'process_type': process_type,
                    'ppid': ppid,
                    'log_file': log_file.name
                })

    return orphaned_processes, all_processes, exp_stats

def print_experiment_descriptions():
    """Print descriptions of the experiments from the source code"""

    try:
        with open("fork_pid.c", 'r') as f:
            source = f.read()

            # Extract experiment descriptions
            pattern = r'{\d+,\s*0,\s*"([A-Z])\\0",\s*"([^"]+)\\0",\s*(\d+),\s*(\d+)}'
            matches = re.findall(pattern, source)

            if matches:
                print("\n=== EXPERIMENT DESCRIPTIONS ===")
                for match in matches:
                    exp_letter, description, sleep_child, sleep_grandchild = match
                    print(f"Experiment {exp_letter}: {description.strip()}")
                    print(f"  - Child sleep: {sleep_child}ms")
                    print(f"  - Grandchild sleep: {sleep_grandchild}ms")
            else:
                print("\nCould not extract experiment descriptions from source code.")
                print("Using default descriptions:")
                print("Experiment B: Sem espera (No Sleep)")
                print("Experiment C: Pai espera (Parent Sleeps)")
                print("Experiment D: Neto órfãos (Grandchild Orphans)")
    except FileNotFoundError:
        print("\nCould not find fork_pid.c file. Using default descriptions:")
        print("Experiment B: Sem espera (No Sleep)")
        print("Experiment C: Pai espera (Parent Sleeps)")
        print("Experiment D: Neto órfãos (Grandchild Orphans)")

def print_orphaned_statistics(exp_stats):
    """Print statistics about orphaned processes by experiment"""

    print("\n=== ORPHANED PROCESSES STATISTICS ===")
    print("| Experiment | Orphaned | Total | Percentage |")
    print("|------------|----------|-------|------------|")

    # Sort by experiment letter
    for exp_letter in sorted(exp_stats.keys()):
        stats = exp_stats[exp_letter]
        if stats['total_g'] > 0:
            percentage = (stats['orphaned_g'] / stats['total_g']) * 100
            print(f"| {exp_letter}          | {stats['orphaned_g']:8d} | {stats['total_g']:5d} | {percentage:10.1f}% |")

    # Identify which experiment had the highest percentage of orphans
    max_orphans = 0
    max_exp = None

    for exp_letter, stats in exp_stats.items():
        if stats['total_g'] > 0:
            percentage = (stats['orphaned_g'] / stats['total_g']) * 100
            if percentage > max_orphans:
                max_orphans = percentage
                max_exp = exp_letter

    if max_exp:
        print(f"\nExperiment {max_exp} had the highest percentage of orphaned processes: {max_orphans:.1f}%")

def print_conclusion(exp_stats):
    """Print conclusion based on the analysis"""

    print("\n=== CONCLUSION ===")
    print("Based on the analysis of the process hierarchies and orphaned processes,")
    print("we can draw the following conclusions:\n")

    # B - No Sleep
    b_pct = 0
    if 'B' in exp_stats and exp_stats['B']['total_g'] > 0:
        b_pct = (exp_stats['B']['orphaned_g'] / exp_stats['B']['total_g']) * 100
        print(f"1. In Experiment B (No Sleep), {b_pct:.1f}% of grandchild processes")
        print("   became orphaned despite no explicit delay. This demonstrates that")
        print("   race conditions can occur naturally due to OS scheduling.")

    # C - Parent Sleep
    c_pct = 0
    if 'C' in exp_stats and exp_stats['C']['total_g'] > 0:
        c_pct = (exp_stats['C']['orphaned_g'] / exp_stats['C']['total_g']) * 100
        print(f"\n2. In Experiment C (Parent Sleep), {c_pct:.1f}% of grandchild processes")
        print("   became orphaned. The sleep in parent processes introduces timing variability.")

        if c_pct < b_pct:
            print("   Interestingly, this is lower than Experiment B, suggesting that the")
            print("   parent sleep may actually reduce orphaning in some circumstances.")
        elif c_pct > b_pct:
            print("   This is higher than Experiment B, suggesting that the parent sleep")
            print("   increases the chance of orphaning.")
        else:
            print("   This is similar to Experiment B, suggesting that the parent sleep")
            print("   doesn't significantly affect orphaning rates.")

    # D - Grandchild Sleep
    d_pct = 0
    if 'D' in exp_stats and exp_stats['D']['total_g'] > 0:
        d_pct = (exp_stats['D']['orphaned_g'] / exp_stats['D']['total_g']) * 100
        print(f"\n3. In Experiment D (Grandchild Sleep), {d_pct:.1f}% of grandchild processes")
        print("   became orphaned. This demonstrates how long-running child processes can")
        print("   become orphaned if their parents terminate first.")

        if d_pct > b_pct and d_pct > c_pct:
            print("   This has the highest orphaning rate, confirming that when a process")
            print("   runs for longer, it's more likely to be orphaned.")

    print("\nThese findings highlight how process hierarchies can be affected by timing")
    print("and execution order, which is crucial to understand when developing multi-process")
    print("applications in Unix-like systems.")

def main():
    # Create output directory if it doesn't exist
    os.makedirs("output", exist_ok=True)

    # Extract process information
    orphaned_processes, all_processes, exp_stats = extract_orphaned_processes()

    # Print basic statistics
    total_processes = sum(len(occurrences) for occurrences in all_processes.values())
    total_orphaned = len(orphaned_processes)

    print("\n=== PROCESS HIERARCHY ANALYSIS ===")
    print(f"Total processes analyzed: {total_processes}")
    print(f"Total orphaned processes: {total_orphaned}")
    print(f"Orphaned process percentage: {(total_orphaned / total_processes) * 100:.1f}%")

    # Print experiment descriptions
    print_experiment_descriptions()

    # Print orphaned process statistics
    print_orphaned_statistics(exp_stats)

    # Print conclusion
    print_conclusion(exp_stats)

    # Save orphaned processes to a file
    with open("output/orphaned_processes.txt", "w") as f:
        f.write("=== ORPHANED PROCESSES ===\n")
        for proc in orphaned_processes:
            f.write(f"Experiment: {proc['exp_letter']} (ID: {proc['exp_id']})\n")
            f.write(f"PID: {proc['pid']}, PPID: {proc['ppid']}\n")
            f.write(f"Log file: {proc['log_file']}\n")
            f.write("-" * 40 + "\n")

    print(f"\nDetailed list of orphaned processes saved to output/orphaned_processes.txt")

if __name__ == "__main__":
    main()
