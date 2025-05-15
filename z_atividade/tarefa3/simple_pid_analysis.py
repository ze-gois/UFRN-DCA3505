#!/usr/bin/python
import os
import re
from collections import defaultdict
from pathlib import Path
import math

def parse_log_file(filepath):
    """Parse log file and extract process information"""
    data = []
    try:
        with open(filepath, 'r') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading file {filepath}: {e}")
        return data

    # Pattern to match log entries
    pattern = r"([A-Z]\d+)\t([A-Z])\t(\d+)\t(\d+)(?:\tsleep for (\d+) seconds|\twokeup|\tEnd\tof experiment|)?"
    matches = re.findall(pattern, content)

    for match in matches:
        exp_id, process_type, pid, ppid = match[0], match[1], int(match[2]), int(match[3])

        # Extract experiment letter and repetition number
        exp_letter = exp_id[0]
        repetition = int(exp_id[1:]) if len(exp_id) > 1 else 0

        # Only add each unique process once to avoid duplicates
        if next((p for p in data if p["process_type"] == process_type and p["pid"] == pid and p["log_file"] == os.path.basename(filepath)), None) is None:
            data.append({
                "exp_id": exp_id,
                "exp_letter": exp_letter,
                "repetition": repetition,
                "process_type": process_type,
                "pid": pid,
                "ppid": ppid,
                "log_file": os.path.basename(filepath)
            })

    return data

def collect_pids_by_experiment_and_type(all_data):
    """Group PIDs by experiment letter, process type, and log file"""
    pids_by_group = defaultdict(lambda: defaultdict(list))

    for proc in all_data:
        # Skip main process
        if proc["process_type"] == "M":
            continue

        # Group by experiment, process type, and log file
        key = (proc["exp_letter"], proc["process_type"])
        pids_by_group[key][proc["log_file"]].append(proc["pid"])

    return pids_by_group

def calculate_statistics(numbers):
    """Calculate basic statistics for a list of numbers"""
    if not numbers:
        return {"mean": 0, "std": 0, "min": 0, "max": 0, "count": 0}

    n = len(numbers)
    mean = sum(numbers) / n

    # Calculate standard deviation
    variance_sum = sum((x - mean) ** 2 for x in numbers)
    std_dev = math.sqrt(variance_sum / n) if n > 0 else 0

    return {
        "mean": mean,
        "std": std_dev,
        "min": min(numbers),
        "max": max(numbers),
        "count": n
    }

def analyze_pid_variance(pids_by_group):
    """Calculate variance statistics for PID distributions"""
    variance_stats = {}

    for (exp_letter, proc_type), log_files in pids_by_group.items():
        # Calculate statistics for each log file
        stats = {}
        for log_file, pids in log_files.items():
            if pids:
                stats[log_file] = calculate_statistics(pids)

        # Store statistics for this experiment/process type
        variance_stats[(exp_letter, proc_type)] = stats

    return variance_stats

def generate_text_histogram(numbers, bins=10, width=50):
    """Generate a simple text-based histogram"""
    if not numbers:
        return "No data available for histogram."

    min_val = min(numbers)
    max_val = max(numbers)
    bin_width = (max_val - min_val) / bins if max_val > min_val else 1

    # Count values in each bin
    bin_counts = [0] * bins
    for num in numbers:
        bin_idx = min(int((num - min_val) / bin_width) if bin_width > 0 else 0, bins - 1)
        bin_counts[bin_idx] += 1

    # Find the maximum count for scaling
    max_count = max(bin_counts)
    scale = width / max_count if max_count > 0 else 1

    # Generate the histogram
    histogram = []
    for i, count in enumerate(bin_counts):
        bin_start = min_val + i * bin_width
        bin_end = min_val + (i + 1) * bin_width
        bar = '#' * int(count * scale)
        histogram.append(f"{bin_start:6.0f}-{bin_end:6.0f} | {bar} ({count})")

    return '\n'.join(histogram)

def print_text_histograms(pids_by_group):
    """Print text-based histograms for PID distributions"""
    for (exp_letter, proc_type), log_files in sorted(pids_by_group.items()):
        print(f"\n=== EXPERIMENT {exp_letter} - {proc_type} PROCESSES ===")

        for log_file, pids in sorted(log_files.items()):
            if not pids:
                continue

            stats = calculate_statistics(pids)
            print(f"\nLog File: {log_file}")
            print(f"Count: {stats['count']} | Mean: {stats['mean']:.1f} | Std Dev: {stats['std']:.1f}")
            print(f"Min: {stats['min']} | Max: {stats['max']}")
            print("\nPID Distribution:")
            print(generate_text_histogram(pids))
            print("-" * 60)

def compare_variance_across_logs(variance_stats):
    """Analyze and compare variance across different log files"""
    print("\n=== PID VARIANCE ANALYSIS ===")
    print("| Experiment | Process | Mean PID | Std Dev | Min | Max | Count |")
    print("|------------|---------|----------|---------|-----|-----|-------|")

    # Calculate overall statistics for each experiment/process type
    for (exp_letter, proc_type), stats in sorted(variance_stats.items()):
        all_means = []
        all_counts = []

        for log_stats in stats.values():
            all_means.append(log_stats['mean'])
            all_counts.append(log_stats['count'])

        if all_means and all_counts:
            # Weighted average for mean
            overall_mean = sum(m * c for m, c in zip(all_means, all_counts)) / sum(all_counts)

            # Estimated standard deviation (simplified)
            overall_std = sum(s['std'] for s in stats.values()) / len(stats)

            overall_min = min(stats[log]['min'] for log in stats)
            overall_max = max(stats[log]['max'] for log in stats)
            overall_count = sum(stats[log]['count'] for log in stats)

            print(f"| {exp_letter}          | {proc_type}       | {overall_mean:8.1f} | {overall_std:7.1f} | {overall_min:3d} | {overall_max:3d} | {overall_count:5d} |")

    print("\nVariance Analysis by Log File:")

    # Calculate coefficient of variation (CV) to compare variance
    for (exp_letter, proc_type), stats in sorted(variance_stats.items()):
        print(f"\nExperiment {exp_letter} - {proc_type} Processes:")
        print("  Log File  |  Mean  | Std Dev |   CV   |  Count  |")
        print("------------|--------|---------|--------|---------|")

        cv_values = []

        for log_file, log_stats in sorted(stats.items()):
            mean = log_stats['mean']
            std = log_stats['std']
            cv = (std / mean) * 100 if mean > 0 else 0
            count = log_stats['count']

            print(f"  {log_file:8s}  | {mean:6.1f} | {std:7.1f} | {cv:6.2f}% | {count:7d} |")
            cv_values.append(cv)

        # Calculate CV of CVs to measure consistency
        if cv_values:
            cv_mean = sum(cv_values) / len(cv_values)
            cv_std = math.sqrt(sum((cv - cv_mean) ** 2 for cv in cv_values) / len(cv_values)) if len(cv_values) > 1 else 0
            cv_of_cvs = (cv_std / cv_mean) * 100 if cv_mean > 0 else 0

            print(f"\n  CV of CVs: {cv_of_cvs:.2f}% - {'Low variance between logs' if cv_of_cvs < 20 else 'High variance between logs'}")

def generate_summary(pids_by_group, variance_stats):
    """Generate a summary file with analysis results"""
    os.makedirs("output", exist_ok=True)

    with open("output/pid_analysis_summary.txt", "w") as f:
        f.write("# PID Distribution Analysis\n\n")

        f.write("## Overview\n\n")
        f.write("This analysis examines the distribution of Process IDs (PIDs) across different ")
        f.write("experiments and process types. Our goal is to determine whether the distribution ")
        f.write("of PIDs remains consistent across different repetitions of the experiments.\n\n")

        f.write("## Key Statistics\n\n")
        f.write("| Experiment | Process | Mean PID | Std Dev | Min | Max | Count |\n")
        f.write("|------------|---------|----------|---------|-----|-----|-------|\n")

        for (exp_letter, proc_type), stats in sorted(variance_stats.items()):
            all_means = []
            all_counts = []

            for log_stats in stats.values():
                all_means.append(log_stats['mean'])
                all_counts.append(log_stats['count'])

            if all_means and all_counts:
                # Weighted average for mean
                overall_mean = sum(m * c for m, c in zip(all_means, all_counts)) / sum(all_counts)

                # Estimated standard deviation (simplified)
                overall_std = sum(s['std'] for s in stats.values()) / len(stats)

                overall_min = min(stats[log]['min'] for log in stats)
                overall_max = max(stats[log]['max'] for log in stats)
                overall_count = sum(stats[log]['count'] for log in stats)

                f.write(f"| {exp_letter} | {proc_type} | {overall_mean:.1f} | {overall_std:.1f} | {overall_min} | {overall_max} | {overall_count} |\n")

        f.write("\n## Variance Analysis\n\n")

        # Calculate coefficient of variation (CV) and compare across logs
        for (exp_letter, proc_type), stats in sorted(variance_stats.items()):
            f.write(f"### Experiment {exp_letter} - {proc_type} Processes\n\n")
            f.write("| Log File | Mean | Std Dev | CV | Count |\n")
            f.write("|----------|------|---------|----|---------|\n")

            cv_values = []

            for log_file, log_stats in sorted(stats.items()):
                mean = log_stats['mean']
                std = log_stats['std']
                cv = (std / mean) * 100 if mean > 0 else 0
                count = log_stats['count']

                f.write(f"| {log_file} | {mean:.1f} | {std:.1f} | {cv:.2f}% | {count} |\n")
                cv_values.append(cv)

            if cv_values:
                cv_mean = sum(cv_values) / len(cv_values)
                cv_std = math.sqrt(sum((cv - cv_mean) ** 2 for cv in cv_values) / len(cv_values)) if len(cv_values) > 1 else 0
                cv_of_cvs = (cv_std / cv_mean) * 100 if cv_mean > 0 else 0

                f.write(f"\nCV of CVs: {cv_of_cvs:.2f}% - ")
                f.write("Low variance between logs\n\n" if cv_of_cvs < 20 else "High variance between logs\n\n")

        f.write("## Conclusion\n\n")
        f.write("The analysis of PID distributions across different log files shows ")

        # Count how many experiment/process combinations have low variance
        low_variance_count = 0
        total_combinations = 0

        for stats in variance_stats.values():
            cv_values = []

            for log_stats in stats.values():
                mean = log_stats['mean']
                std = log_stats['std']
                cv = (std / mean) * 100 if mean > 0 else 0
                cv_values.append(cv)

            if cv_values:
                total_combinations += 1
                cv_mean = sum(cv_values) / len(cv_values)
                cv_std = math.sqrt(sum((cv - cv_mean) ** 2 for cv in cv_values) / len(cv_values)) if len(cv_values) > 1 else 0
                cv_of_cvs = (cv_std / cv_mean) * 100 if cv_mean > 0 else 0

                if cv_of_cvs < 20:
                    low_variance_count += 1

        if total_combinations > 0:
            consistency_pct = (low_variance_count / total_combinations) * 100

            if consistency_pct >= 75:
                f.write("strong consistency in the variance across repetitions. ")
                f.write("This indicates that the process creation behavior is highly reproducible ")
                f.write("and the experimental conditions remain stable across different runs.\n\n")
            elif consistency_pct >= 50:
                f.write("moderate consistency in the variance across repetitions. ")
                f.write("While there are some variations, the general patterns of process creation ")
                f.write("appear to be maintained across different runs.\n\n")
            else:
                f.write("limited consistency in the variance across repetitions. ")
                f.write("This suggests that the process creation patterns may be influenced ")
                f.write("by variable factors in the system environment.\n\n")

        f.write("The PID distributions and their variance characteristics provide valuable ")
        f.write("context for understanding the behavior of orphaned processes observed in ")
        f.write("the different experiments.\n")

def main():
    # Check if log directory exists
    if not os.path.isdir("log"):
        print("Error: 'log' directory not found. Make sure you're in the right working directory.")
        return

    # Parse all log files
    log_files = list(Path("log").glob("*.log"))
    if not log_files:
        print("Error: No log files found in the 'log' directory")
        return

    print(f"Analyzing {len(log_files)} log files...")

    all_data = []
    for log_file in log_files:
        all_data.extend(parse_log_file(log_file))

    # Group PIDs by experiment letter, process type, and log file
    pids_by_group = collect_pids_by_experiment_and_type(all_data)

    # Calculate variance statistics
    variance_stats = analyze_pid_variance(pids_by_group)

    # Print text histograms
    print_text_histograms(pids_by_group)

    # Analyze and compare variance
    compare_variance_across_logs(variance_stats)

    # Generate summary
    generate_summary(pids_by_group, variance_stats)

    print("\nAnalysis complete!")
    print("- Summary saved to output/pid_analysis_summary.txt")

if __name__ == "__main__":
    main()
