#!/usr/bin/python
import os
import math
from collections import defaultdict
from typing import Dict, List, Any, Tuple

from .a00_parser import parse_all_logs, get_log_files


def count_occurrences(data: List[Dict[str, Any]], group_by: List[str]) -> Dict[Tuple, int]:
    """Count occurrences of items grouped by specified fields.
    
    Args:
        data: List of data dictionaries
        group_by: List of fields to group by
        
    Returns:
        Dictionary with tuples of group values as keys and counts as values
    """
    counts = defaultdict(int)
    for item in data:
        # Create tuple of values for the group_by fields
        key = tuple(item.get(field) for field in group_by)
        counts[key] += 1
    
    return counts


def identify_orphaned_processes(data: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
    """Identify orphaned processes (PPID = 1) from the data.
    
    Args:
        data: List of process data dictionaries
        
    Returns:
        List of orphaned process data dictionaries
    """
    return [item for item in data if item["ppid"] == 1 and item["process_type"] == "G"]


def calculate_statistics(data: List[Dict[str, Any]], field: str) -> Dict[str, Any]:
    """Calculate basic statistics for a numeric field in the data.
    
    Args:
        data: List of data dictionaries
        field: Field name to calculate statistics for
        
    Returns:
        Dictionary of statistics
    """
    if not data:
        return {
            "mean": 0,
            "median": 0,
            "min": 0,
            "max": 0,
            "std": 0,
            "count": 0
        }
    
    values = [item[field] for item in data]
    n = len(values)
    mean = sum(values) / n
    
    # Sort for median and min/max
    sorted_values = sorted(values)
    if n % 2 == 0:
        median = (sorted_values[n//2 - 1] + sorted_values[n//2]) / 2
    else:
        median = sorted_values[n//2]
    
    # Calculate standard deviation
    variance_sum = sum((x - mean) ** 2 for x in values)
    std_dev = math.sqrt(variance_sum / n) if n > 0 else 0
    
    return {
        "mean": mean,
        "median": median,
        "min": sorted_values[0],
        "max": sorted_values[-1],
        "std": std_dev,
        "count": n
    }


def analyze_orphaned_processes(data: List[Dict[str, Any]]) -> Dict[str, Any]:
    """Analyze orphaned processes and calculate statistics by experiment.
    
    Args:
        data: List of process data dictionaries
        
    Returns:
        Dictionary containing orphan analysis results
    """
    # Get orphaned grandchildren
    orphaned_gcs = identify_orphaned_processes(data)
    
    # Count total grandchildren by experiment type
    total_grandchildren = defaultdict(int)
    for item in data:
        if item["process_type"] == "G":
            total_grandchildren[item["exp_letter"]] += 1
    
    # Count orphaned grandchildren by experiment type
    orphaned_counts = defaultdict(int)
    for item in orphaned_gcs:
        orphaned_counts[item["exp_letter"]] += 1
    
    # Calculate percentages
    orphan_percentages = {}
    orphan_results = {}
    
    for exp_letter in sorted(total_grandchildren.keys()):
        total = total_grandchildren[exp_letter]
        orphaned = orphaned_counts.get(exp_letter, 0)
        
        if total > 0:
            percentage = (orphaned / total) * 100
        else:
            percentage = 0
            
        orphan_percentages[exp_letter] = percentage
        orphan_results[exp_letter] = {
            "orphan_count": orphaned,
            "total_count": total,
            "percentage": percentage
        }
    
    return {
        "orphan_counts": orphaned_counts,
        "total_counts": total_grandchildren,
        "orphan_percentages": orphan_percentages,
        "orphaned_processes": orphaned_gcs,
        "by_experiment": orphan_results
    }


def analyze_pid_variance(data: List[Dict[str, Any]]) -> Dict[str, Any]:
    """Analyze variance in process IDs across different log files.
    
    Args:
        data: List of process data dictionaries
        
    Returns:
        Dictionary containing PID variance analysis results
    """
    # Group data by experiment, process type, and log file
    grouped_data = defaultdict(lambda: defaultdict(list))
    
    for item in data:
        if item["process_type"] == "M":
            continue  # Skip main process
            
        key = (item["exp_letter"], item["process_type"])
        grouped_data[key][item["log_file"]].append(item["pid"])
    
    # Calculate statistics for each group
    variance_stats = {}
    
    for (exp_letter, proc_type), log_files in grouped_data.items():
        stats = {}
        for log_file, pids in log_files.items():
            if pids:
                stats[log_file] = calculate_statistics(
                    [{"pid": pid} for pid in pids], 
                    "pid"
                )
        
        variance_stats[(exp_letter, proc_type)] = stats
    
    # Calculate coefficient of variation (CV) to compare variance
    cv_analysis = {}
    
    for (exp_letter, proc_type), stats in variance_stats.items():
        cv_values = []
        
        for log_file, log_stats in stats.items():
            mean = log_stats['mean']
            std = log_stats['std']
            cv = (std / mean) * 100 if mean > 0 else 0
            cv_values.append(cv)
        
        if cv_values:
            cv_mean = sum(cv_values) / len(cv_values)
            cv_std = math.sqrt(sum((cv - cv_mean) ** 2 for cv in cv_values) / len(cv_values)) if len(cv_values) > 1 else 0
            cv_of_cvs = (cv_std / cv_mean) * 100 if cv_mean > 0 else 0
            
            cv_analysis[(exp_letter, proc_type)] = {
                "cv_values": cv_values,
                "cv_mean": cv_mean,
                "cv_std": cv_std,
                "cv_of_cvs": cv_of_cvs,
                "is_consistent": cv_of_cvs < 20  # Low variance if CV of CVs < 20%
            }
    
    return {
        "variance_stats": variance_stats,
        "cv_analysis": cv_analysis
    }


def analyze_sleep_patterns(data: List[Dict[str, Any]]) -> Dict[str, Any]:
    """Analyze sleep and wakeup patterns in the data.
    
    Args:
        data: List of process data dictionaries
        
    Returns:
        Dictionary containing sleep pattern analysis results
    """
    # Filter only sleep and wakeup events
    sleep_events = [item for item in data if item["action"] == "sleep"]
    wakeup_events = [item for item in data if item["action"] == "wakeup"]
    
    # Group by experiment letter and process type
    sleep_durations = {}
    for item in sleep_events:
        key = (item["exp_letter"], item["process_type"])
        if key not in sleep_durations and item["sleep_duration"] > 0:
            sleep_durations[key] = item["sleep_duration"]
    
    # Count sleep and wakeup events by experiment and process type
    sleep_counts = count_occurrences(sleep_events, ["exp_letter", "process_type"])
    wakeup_counts = count_occurrences(wakeup_events, ["exp_letter", "process_type"])
    
    return {
        "sleep_durations": sleep_durations,
        "sleep_counts": sleep_counts,
        "wakeup_counts": wakeup_counts,
        "sleep_events": sleep_events,
        "wakeup_events": wakeup_events
    }


def analyze_process_hierarchy(data: List[Dict[str, Any]]) -> Dict[str, Any]:
    """Analyze process hierarchy relationships in the data.
    
    Args:
        data: List of process data dictionaries
        
    Returns:
        Dictionary containing process hierarchy analysis results
    """
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
    
    # Analyze parent-child relationships
    parent_child_map = {}
    for item in data:
        pid = item["pid"]
        ppid = item["ppid"]
        
        if pid not in parent_child_map:
            parent_child_map[pid] = {
                "type": item["process_type"],
                "exp_letter": item["exp_letter"],
                "parent": ppid,
                "children": []
            }
        
        # Add this process as a child of its parent
        if ppid in parent_child_map:
            if pid not in parent_child_map[ppid]["children"]:
                parent_child_map[ppid]["children"].append(pid)
    
    return {
        "unique_processes": unique_processes,
        "hierarchy_stats": hierarchy_stats,
        "parent_child_map": parent_child_map
    }


if __name__ == "__main__":
    # Test the statistical analysis
    print("Testing statistical analysis...")
    log_files = get_log_files(limit=3)  # Limit to 3 files for testing
    data = parse_all_logs(log_files, verbose=True)
    
    # Run analysis
    orphan_analysis = analyze_orphaned_processes(data)
    print("\nOrphaned Processes Analysis:")
    for exp_letter, stats in orphan_analysis["by_experiment"].items():
        print(f"  Experiment {exp_letter}: {stats['orphan_count']}/{stats['total_count']} ({stats['percentage']:.1f}%)")
    
    # Analyze PID variance
    pid_variance = analyze_pid_variance(data)
    print("\nPID Variance Analysis:")
    for (exp_letter, proc_type), cv_data in pid_variance["cv_analysis"].items():
        consistency = "Consistent" if cv_data["is_consistent"] else "Inconsistent"
        print(f"  Experiment {exp_letter} - {proc_type}: CV of CVs = {cv_data['cv_of_cvs']:.2f}% ({consistency})")
    
    print("\nAnalysis complete.")