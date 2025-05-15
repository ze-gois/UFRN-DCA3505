#!/usr/bin/python
import os
import re
from pathlib import Path
from typing import Dict, List, Any, Optional

def parse_log_file(filepath: str) -> List[Dict[str, Any]]:
    """Parse a log file and extract process information.

    Args:
        filepath: Path to the log file to parse

    Returns:
        List of dictionaries containing structured log data
    """
    data = []
    try:
        with open(filepath, 'r') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading file {filepath}: {e}")
        return data

    # Pattern to match log entries
    pattern = r"([A-Z]\d+)\t([A-Z])\t(\d+)\t(\d+)(?:\tsleep for (\d+) seconds|\twokeup|\tEnd\tof experiment|)?"
    # pattern = r"([A-Z]\d+)\t([A-Z])\t(\d+)\t(\d+)\t()$"
    pattern = r"([A-Z]\d+)\t([A-Z])\t(\d+)\t(\d+)(?:\t(sleep for \d+ seconds|wokeup|End\tof experiment))?"
    matches = re.findall(pattern, content)


    for m,match in enumerate(matches):
        exp_id, process_type, pid, ppid = match[0], match[1], int(match[2]), int(match[3])
        sleep_duration = -2

        # Extract experiment letter and repetition number
        exp_letter = exp_id[0]
        repetition = int(exp_id[1:]) if len(exp_id) > 1 else 0

        # Determine action type
        if "sleep" in str(match):
            action = "sleep"
            sleep_duration = int(re.findall(r"\d+",match[4])[0])
        elif "wokeup" in str(match):
            action = "wakeup"
            sleep_duration = -1
        elif "End" in str(match):
            action = "end"
        else:
            action = "other"

        entry = {
            "exp_id": exp_id,
            "exp_letter": exp_letter,
            "repetition": repetition,
            "process_type": process_type,
            "pid": pid,
            "ppid": ppid,
            "sleep_duration": sleep_duration,
            "action": action,
            "log_file": os.path.basename(filepath)
        }

        data.append(entry)

    return data

def get_log_files(log_dir: str="log", pattern: str="*.log", limit: Optional[int]=None) -> List[str]:
    """Get list of log files in directory matching pattern.

    Args:
        log_dir: Directory to search for log files
        pattern: Glob pattern to match log files
        limit: Optional limit on number of files to return

    Returns:
        List of paths to log files
    """
    log_files = sorted(list(Path(log_dir).glob(pattern)))
    if limit is not None:
        log_files = log_files[:limit]
    return [str(path) for path in log_files]

def parse_all_logs(log_files: List[str], verbose: bool=False) -> List[Dict[str, Any]]:
    """Parse all log files in the provided list.

    Args:
        log_files: List of log file paths
        verbose: Whether to print progress information

    Returns:
        Combined list of parsed log data
    """
    if verbose:
        print(f"Parsing {len(log_files)} log files...")

    all_data = []
    for i, log_file in enumerate(log_files):
        all_data.extend(parse_log_file(log_file))
        if verbose and (i + 1) % 50 == 0:
            print(f"  Processed {i + 1}/{len(log_files)} log files...")

    if verbose:
        print(f"Parsed {len(all_data)} log entries from {len(log_files)} files")

    return all_data

def extract_experiment_descriptions(filepath: str="fork_pid.c") -> List[Dict[str, Any]]:
    """Extract experiment descriptions from the source code.

    Args:
        filepath: Path to the source code file

    Returns:
        List of dictionaries containing experiment descriptions
    """
    descriptions = []

    try:
        with open(filepath, 'r') as f:
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

def get_df():
    import polars as pl
    df_path = os.path.join(os.getcwd(),'log',"0.df")
    if Path(df_path).exists():
        return pl.read_ipc("./log/0.df")
    else:
        log_files = get_log_files(limit=None)  # Limit to 3 for testing
        data = parse_all_logs(log_files, verbose=True)

        df = pl.DataFrame(data)
        # Salvar no formato Arrow IPC (Feather)
        df.write_ipc("./log/0.df")  # extensão pode ser qualquer uma
        return df

if __name__ == "__main__":
    # Test the parser
    import sys
    if len(sys.argv) > 1:
        file_to_parse = sys.argv[1]
        data = parse_log_file(file_to_parse)
        print(f"Parsed {len(data)} entries from {file_to_parse}")
        # Print first entry as example
        if data:
            print("Example entry:")
            for k, v in data[0].items():
                print(f"  {k}: {v}")
    else:
        # Parse all log files as a test
        log_files = get_log_files(limit=None)  # Limit to 3 for testing
        all_data = parse_all_logs(log_files, verbose=True)
        print(set([a['action'] for a in all_data]))
        print(f"Total parsed entries: {len(all_data)}")
