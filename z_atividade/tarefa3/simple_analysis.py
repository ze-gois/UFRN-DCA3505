#!/usr/bin/python
import os
import re
import csv
from collections import defaultdict, Counter
from pathlib import Path

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
        sleep_duration = int(match[4]) if match[4] and match[4].isdigit() else 0

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

    return data

def analyze_parent_child_relationships(data):
    """Analyze parent-child relationships in the process hierarchy."""
    # Count instances where grandchild's ppid is 1 (orphaned)
    orphaned_gcs = [item for item in data if item["process_type"] == "G" and item["ppid"] == 1]

    # Group by experiment type
    orphan_counts = defaultdict(int)
    for item in orphaned_gcs:
        orphan_counts[item["exp_letter"]] += 1

    # Count total grandchildren by experiment type
    total_gcs = defaultdict(int)
    for item in data:
        if item["process_type"] == "G":
            total_gcs[item["exp_letter"]] += 1

    # Calculate percentages
    orphan_percentages = {}
    for exp_letter in total_gcs:
        if total_gcs[exp_letter] > 0:
            orphan_percentages[exp_letter] = (orphan_counts.get(exp_letter, 0) / total_gcs[exp_letter]) * 100
        else:
            orphan_percentages[exp_letter] = 0

    return {
        "orphan_counts": orphan_counts,
        "total_counts": total_gcs,
        "orphan_percentages": orphan_percentages
    }

def analyze_sleep_durations(data):
    """Analyze sleep durations by experiment type and process type."""
    # Filter only sleep events
    sleep_events = [item for item in data if item["action"] == "sleep"]

    # Group by experiment letter and process type
    sleep_durations = {}
    for item in sleep_events:
        key = (item["exp_letter"], item["process_type"])
        if key not in sleep_durations and item["sleep_duration"] > 0:
            sleep_durations[key] = item["sleep_duration"]

    return sleep_durations

def analyze_process_hierarchies(data):
    """Analyze process hierarchies and their stability."""
    # Count unique processes by type and experiment
    unique_processes = defaultdict(lambda: defaultdict(set))
    for item in data:
        unique_processes[item["exp_letter"]][item["process_type"]].add(item["pid"])

    # Convert sets to counts
    hierarchy_stats = {}
    for exp_letter, process_types in unique_processes.items():
        hierarchy_stats[exp_letter] = {
            process_type: len(pids) for process_type, pids in process_types.items()
        }

    return hierarchy_stats

def extract_experiment_descriptions():
    """Extract experiment descriptions from the source code."""
    descriptions = []

    try:
        with open("fork_pid.c", 'r') as f:
            source = f.read()

        # Extract experiment descriptions
        pattern = r'{\d+,\s*0,\s*"([A-Z])\\0",\s*"([^"]+)\\0",\s*(\d+),\s*(\d+)}'
        matches = re.findall(pattern, source)

        for match in matches:
            exp_letter, description, sleep_child, sleep_grandchild = match
            descriptions.append({
                "exp_letter": exp_letter,
                "description": description.strip(),
                "sleep_child": int(sleep_child),
                "sleep_grandchild": int(sleep_grandchild)
            })
    except FileNotFoundError:
        # If we can't find the file, use hardcoded values from what we observed
        descriptions = [
            {"exp_letter": "B", "description": "Experiment B - Sem espera", "sleep_child": 0, "sleep_grandchild": 0},
            {"exp_letter": "C", "description": "Experiment C - Pai espera", "sleep_child": 100, "sleep_grandchild": 0},
            {"exp_letter": "D", "description": "Experiment D - Neto órfãos", "sleep_child": 0, "sleep_grandchild": 100}
        ]

    return descriptions

def generate_summary_report(data, orphan_analysis, sleep_durations, hierarchy_stats):
    """Generate a summary text report of the analysis findings"""
    # Ensure output directory exists
    os.makedirs("output", exist_ok=True)

    with open("output/analysis_summary.txt", "w") as f:
        f.write("# Process Hierarchy and Orphan Analysis\n\n")

        # Experiment descriptions
        f.write("## Experiment Descriptions\n\n")
        descriptions = extract_experiment_descriptions()
        for desc in descriptions:
            f.write(f"- **Experiment {desc['exp_letter']}**: {desc['description']} ")
            f.write(f"(Child sleep: {desc['sleep_child']}ms, Grandchild sleep: {desc['sleep_grandchild']}ms)\n")

        f.write("\n")

        # Orphan analysis
        f.write("## Orphaned Process Analysis\n\n")

        for exp_letter in sorted(orphan_analysis["total_counts"].keys()):
            total = orphan_analysis["total_counts"][exp_letter]
            orphaned = orphan_analysis["orphan_counts"].get(exp_letter, 0)
            percentage = orphan_analysis["orphan_percentages"].get(exp_letter, 0)

            f.write(f"- **Experiment {exp_letter}**: ")
            f.write(f"{orphaned} orphaned grandchildren out of {total} ")
            f.write(f"({percentage:.1f}%)\n")

        f.write("\n")

        # Sleep durations
        f.write("## Sleep Durations\n\n")
        for (exp_letter, proc_type), duration in sorted(sleep_durations.items()):
            f.write(f"- **Experiment {exp_letter} ({proc_type} processes)**: {duration} seconds\n")

        f.write("\n")

        # Process hierarchy stats
        f.write("## Process Hierarchy Statistics\n\n")

        # Group by experiment letter
        for exp_letter in sorted(hierarchy_stats.keys()):
            f.write(f"### Experiment {exp_letter}\n\n")

            for proc_type, count in hierarchy_stats[exp_letter].items():
                f.write(f"- **{proc_type} processes**: {count} unique processes\n")

            f.write("\n")

        # Conclusion
        f.write("## Conclusions\n\n")
        f.write("Based on the analysis of the log files, we can observe distinct behaviors in each experiment:\n\n")

        # Experiment B - Sem espera
        b_pct = orphan_analysis["orphan_percentages"].get("B", 0)
        f.write(f"- **Experiment B (No Sleep)**: Without explicit sleep, we observed {b_pct:.1f}% orphaned grandchildren. ")
        f.write("Race conditions can occur but are less predictable.\n\n")

        # Experiment C - Pai espera
        c_pct = orphan_analysis["orphan_percentages"].get("C", 0)
        f.write(f"- **Experiment C (Parent Sleeps)**: When parent processes sleep, we observed {c_pct:.1f}% orphaned grandchildren. ")
        f.write("The sleep in parent processes introduces timing variability.\n\n")

        # Experiment D - Neto órfãos
        d_pct = orphan_analysis["orphan_percentages"].get("D", 0)
        f.write(f"- **Experiment D (Grandchild Orphans)**: When grandchildren sleep, we observed {d_pct:.1f}% orphaned grandchildren. ")
        f.write("This demonstrates how long-running child processes can become orphaned if their parents terminate first.\n")

def save_as_csv(data, filename):
    """Save data as CSV file"""
    os.makedirs("output", exist_ok=True)

    with open(f"output/{filename}", 'w', newline='') as f:
        if not data:
            return

        # Get fieldnames from the first record
        fieldnames = data[0].keys()
        writer = csv.DictWriter(f, fieldnames=fieldnames)

        writer.writeheader()
        writer.writerows(data)

def process_pattern_counter(data):
    """Count processes by their type and experiment"""
    counter = Counter()
    for item in data:
        counter[(item["exp_letter"], item["process_type"])] += 1

    # Format for reporting
    result = []
    for (exp_letter, proc_type), count in sorted(counter.items()):
        result.append({
            "exp_letter": exp_letter,
            "process_type": proc_type,
            "count": count
        })

    return result

def main():
    # Parse all log files
    log_files = [str(p) for p in Path("log").glob("*.log")]
    all_data = []

    for log_file in log_files:
        all_data.extend(parse_log_file(log_file))

    # Save raw parsed data
    save_as_csv(all_data, "parsed_logs.csv")

    # Analyze parent-child relationships
    orphan_analysis = analyze_parent_child_relationships(all_data)

    # Analyze sleep durations
    sleep_durations = analyze_sleep_durations(all_data)

    # Analyze process hierarchies
    hierarchy_stats = analyze_process_hierarchies(all_data)

    # Count processes by type
    process_counts = process_pattern_counter(all_data)
    save_as_csv(process_counts, "process_counts.csv")

    # Generate summary report
    generate_summary_report(all_data, orphan_analysis, sleep_durations, hierarchy_stats)

    print("Analysis complete! Results saved to output/ directory")
    print(f"Processed {len(all_data)} log entries from {len(log_files)} files")

    # Print orphaned processes summary to console
    print("\nOrphaned Grandchildren Summary:")
    for exp_letter in sorted(orphan_analysis["orphan_percentages"].keys()):
        pct = orphan_analysis["orphan_percentages"][exp_letter]
        total = orphan_analysis["total_counts"][exp_letter]
        orphaned = orphan_analysis["orphan_counts"].get(exp_letter, 0)
        print(f"  Experiment {exp_letter}: {orphaned}/{total} ({pct:.1f}%)")

if __name__ == "__main__":
    main()
