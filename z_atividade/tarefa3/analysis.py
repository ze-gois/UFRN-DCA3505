#!/usr/bin/python
import os
import re
import polars as pl
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path

# Set the style for the plots
plt.style.use('seaborn-v0_8-whitegrid')
plt.rcParams.update({
    'font.size': 12,
    'figure.figsize': (12, 7),
    'axes.titlesize': 16,
    'axes.labelsize': 14
})

# Ensure output directories exist
os.makedirs("./output", exist_ok=True)
os.makedirs("./figs", exist_ok=True)

def parse_log_file(filepath):
    """Parse log file and extract process information"""
    data = []
    with open(filepath, 'r') as f:
        content = f.read()

    # Pattern to match log entries
    pattern = r"([A-Z]\d+)\t([A-Z])\t(\d+)\t(\d+)(?:\tsleep for (\d+) seconds|\twokeup|\tEnd\tof experiment|)?"
    matches = re.findall(pattern, content)

    for match in matches:
        exp_id, process_type, pid, ppid = match[0], match[1], int(match[2]), int(match[3])
        sleep_duration = int(match[4]) if match[4] else 0

        # Extract experiment letter and repetition number
        exp_letter = exp_id[0]
        repetition = int(exp_id[1:]) if len(exp_id) > 1 else 0

        # Determine action type
        if "sleep" in str(match):
            action = "sleep"
        elif "wokeup" in str(match):
            action = "wakeup"
        elif "End" in str(match):
            action = "end"
        else:
            action = "other"

        data.append({
            "exp_id": exp_id,
            "exp_letter": exp_letter,
            "repetition": repetition,
            "process_type": process_type,
            "pid": pid,
            "ppid": ppid,
            "sleep_duration": sleep_duration,
            "action": action,
            "log_file": os.path.basename(filepath)
        })

    return pl.DataFrame(data)

def analyze_parent_child_relationships(df):
    """Analyze parent-child relationships in the process hierarchy."""
    # Count instances where grandchild's ppid is 1 (orphaned)
    orphaned_gcs = df.filter((df["process_type"] == "G") & (df["ppid"] == 1))

    # Analyze by experiment type
    orphan_counts = orphaned_gcs.group_by("exp_letter").agg(
        pl.count().alias("orphan_count")
    ).sort("exp_letter")

    total_gcs = df.filter(df["process_type"] == "G").group_by("exp_letter").agg(
        pl.count().alias("total_count")
    ).sort("exp_letter")

    result = orphan_counts.join(total_gcs, on="exp_letter", how="outer")
    result = result.with_columns([
        (pl.col("orphan_count") / pl.col("total_count") * 100).alias("orphan_percentage")
    ])

    return result

def plot_orphaned_grandchildren(orphan_analysis):
    """Plot the percentage of orphaned grandchildren by experiment."""
    exp_letters = orphan_analysis["exp_letter"].to_list()
    percentages = orphan_analysis["orphan_percentage"].to_list()

    plt.figure(figsize=(10, 6))
    bars = plt.bar(exp_letters, percentages, color='skyblue')

    # Add percentage labels on top of bars
    for bar, percentage in zip(bars, percentages):
        plt.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 1,
                 f'{percentage:.1f}%', ha='center', va='bottom')

    plt.title('Percentage of Orphaned Grandchildren by Experiment Type')
    plt.xlabel('Experiment Type')
    plt.ylabel('Percentage of Orphaned Grandchildren')
    plt.ylim(0, 100)
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.savefig("./figs/orphaned_grandchildren.png", dpi=300, bbox_inches='tight')
    plt.close()

def analyze_sleep_wakeup_patterns(df):
    """Analyze sleep and wakeup patterns for processes."""
    # Filter only sleep and wakeup events
    sleep_wakeup = df.filter(pl.col("action").is_in(["sleep", "wakeup"]))

    # Group by experiment type and process type
    patterns = sleep_wakeup.group_by(["exp_letter", "process_type", "sleep_duration"]).agg(
        pl.count().alias("event_count")
    ).sort(["exp_letter", "process_type"])

    return patterns

def plot_sleep_durations(sleep_patterns):
    """Plot sleep durations by experiment type and process type."""
    # Filter to just get unique sleep durations per experiment/process type
    sleep_durations = sleep_patterns.filter(
        pl.col("sleep_duration") > 0
    ).unique(subset=["exp_letter", "process_type", "sleep_duration"])

    # Pivot for plotting
    plot_data = sleep_durations.pivot(
        index="exp_letter",
        columns="process_type",
        values="sleep_duration",
        aggregate_function="first"
    )

    plt.figure(figsize=(10, 6))

    # Get experiment types
    exp_types = plot_data["exp_letter"].to_list()

    # Bar positions
    x = np.arange(len(exp_types))
    width = 0.35

    # Plot bars for C (child) and G (grandchild) processes
    c_durations = [d if d is not None else 0 for d in plot_data["C"].to_list()]
    g_durations = [d if d is not None else 0 for d in plot_data["G"].to_list()]

    plt.bar(x - width/2, c_durations, width, label='Child Process')
    plt.bar(x + width/2, g_durations, width, label='Grandchild Process')

    plt.xlabel('Experiment Type')
    plt.ylabel('Sleep Duration (seconds)')
    plt.title('Sleep Durations by Experiment Type and Process Type')
    plt.xticks(x, exp_types)
    plt.legend()
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.savefig("./figs/sleep_durations.png", dpi=300, bbox_inches='tight')
    plt.close()

def analyze_process_hierarchies(df):
    """Analyze process hierarchies and their stability."""
    # Group by experiment letter and calculate statistics
    hierarchy_stats = df.group_by(["exp_letter", "process_type"]).agg([
        pl.n_unique("pid").alias("unique_processes"),
        pl.count().alias("total_events")
    ]).sort(["exp_letter", "process_type"])

    return hierarchy_stats

def plot_process_hierarchies(hierarchy_stats):
    """Plot process hierarchy statistics."""
    # Filter to just get the unique processes count
    unique_processes = hierarchy_stats.filter(
        pl.col("process_type").is_in(["C", "G"])
    ).pivot(
        index="exp_letter",
        columns="process_type",
        values="unique_processes"
    )

    plt.figure(figsize=(10, 6))

    # Get experiment types
    exp_types = unique_processes["exp_letter"].to_list()

    # Bar positions
    x = np.arange(len(exp_types))
    width = 0.35

    # Plot bars for C (child) and G (grandchild) processes
    c_counts = unique_processes["C"].to_list()
    g_counts = unique_processes["G"].to_list()

    plt.bar(x - width/2, c_counts, width, label='Child Processes')
    plt.bar(x + width/2, g_counts, width, label='Grandchild Processes')

    plt.xlabel('Experiment Type')
    plt.ylabel('Number of Unique Processes')
    plt.title('Process Hierarchy by Experiment Type')
    plt.xticks(x, exp_types)
    plt.legend()
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.savefig("./figs/process_hierarchies.png", dpi=300, bbox_inches='tight')
    plt.close()

def analyze_experiment_descriptions():
    """Extract and analyze experiment descriptions from the source code."""
    with open("./fork_pid.c", 'r') as f:
        source = f.read()

    # Extract experiment descriptions
    pattern = r'{\d+,\s*0,\s*"([A-Z])\\0",\s*"([^"]+)\\0",\s*(\d+),\s*(\d+)}'
    matches = re.findall(pattern, source)

    data = []
    for match in matches:
        exp_letter, description, sleep_child, sleep_grandchild = match
        data.append({
            "exp_letter": exp_letter,
            "description": description.strip(),
            "sleep_child": int(sleep_child),
            "sleep_grandchild": int(sleep_grandchild)
        })

    return pl.DataFrame(data)

def generate_summary_report(df, orphan_analysis, hierarchy_stats):
    """Generate a summary text report of the analysis findings"""
    with open("./output/analysis_summary.txt", "w") as f:
        f.write("# Process Hierarchy and Orphan Analysis\n\n")

        # Experiment descriptions
        exp_descriptions = analyze_experiment_descriptions()
        f.write("## Experiment Descriptions\n\n")
        for row in exp_descriptions.to_dicts():
            f.write(f"- **Experiment {row['exp_letter']}**: {row['description']} ")
            f.write(f"(Child sleep: {row['sleep_child']}ms, Grandchild sleep: {row['sleep_grandchild']}ms)\n")

        f.write("\n")

        # Orphan analysis
        f.write("## Orphaned Process Analysis\n\n")
        for row in orphan_analysis.to_dicts():
            f.write(f"- **Experiment {row['exp_letter']}**: ")
            f.write(f"{row['orphan_count']} orphaned grandchildren out of {row['total_count']} ")
            f.write(f"({row['orphan_percentage']:.1f}%)\n")

        f.write("\n")

        # Process hierarchy stats
        f.write("## Process Hierarchy Statistics\n\n")

        # Group by experiment letter
        for exp in sorted(hierarchy_stats["exp_letter"].unique()):
            f.write(f"### Experiment {exp}\n\n")
            exp_data = hierarchy_stats.filter(pl.col("exp_letter") == exp)

            for row in exp_data.to_dicts():
                f.write(f"- **{row['process_type']} processes**: ")
                f.write(f"{row['unique_processes']} unique processes with {row['total_events']} events\n")

            f.write("\n")

        # Conclusion
        f.write("## Conclusions\n\n")
        f.write("Based on the analysis of the log files, we can observe distinct behaviors in each experiment:\n\n")

        # Experiment B - Sem espera
        f.write("- **Experiment B (No Sleep)**: Without explicit sleep, ")
        b_orphan = orphan_analysis.filter(pl.col("exp_letter") == "B")
        if not b_orphan.is_empty():
            b_pct = b_orphan.select("orphan_percentage")[0, 0]
            f.write(f"we observed {b_pct:.1f}% orphaned grandchildren. ")
        f.write("Race conditions can occur but are less predictable.\n\n")

        # Experiment C - Pai espera
        f.write("- **Experiment C (Parent Sleeps)**: When parent processes sleep, ")
        c_orphan = orphan_analysis.filter(pl.col("exp_letter") == "C")
        if not c_orphan.is_empty():
            c_pct = c_orphan.select("orphan_percentage")[0, 0]
            f.write(f"we observed {c_pct:.1f}% orphaned grandchildren. ")
        f.write("The sleep in parent processes introduces timing variability.\n\n")

        # Experiment D - Neto órfãos
        f.write("- **Experiment D (Grandchild Orphans)**: When grandchildren sleep, ")
        d_orphan = orphan_analysis.filter(pl.col("exp_letter") == "D")
        if not d_orphan.is_empty():
            d_pct = d_orphan.select("orphan_percentage")[0, 0]
            f.write(f"we observed {d_pct:.1f}% orphaned grandchildren. ")
        f.write("This demonstrates how long-running child processes can become orphaned if their parents terminate first.\n")

def main():
    # Parse all log files
    log_files = list(Path("./log").glob("*.log"))
    dfs = [parse_log_file(log_file) for log_file in log_files]
    all_data = pl.concat(dfs)

    # Save raw parsed data
    all_data.write_csv("./output/parsed_logs.csv")

    # Analyze parent-child relationships
    orphan_analysis = analyze_parent_child_relationships(all_data)
    plot_orphaned_grandchildren(orphan_analysis)

    # Analyze sleep patterns
    sleep_patterns = analyze_sleep_wakeup_patterns(all_data)
    plot_sleep_durations(sleep_patterns)

    # Analyze process hierarchies
    hierarchy_stats = analyze_process_hierarchies(all_data)
    plot_process_hierarchies(hierarchy_stats)

    # Generate summary report
    generate_summary_report(all_data, orphan_analysis, hierarchy_stats)

    print("Analysis complete! Results saved to ./output/ and figures to ./figs/")

if __name__ == "__main__":
    main()
