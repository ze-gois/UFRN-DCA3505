import os
import re
import numpy as np
import matplotlib.pyplot as plt
from collections import defaultdict, Counter
import pandas as pd

class PSAnalyzer:
    def __init__(self, log_dir="./log"):
        self.log_dir = log_dir
        self.logs_by_part = self.categorize_logs()
        self.data = self.parse_all_logs()

    def categorize_logs(self):
        """Categorize log files by experiment part (1, 2, 3, 4)"""
        logs_by_part = defaultdict(list)
        
        for filename in sorted(os.listdir(self.log_dir)):
            if not filename.endswith('.log'):
                continue
            
            # Extract the part number (first character of the filename)
            part = filename[0]
            if part in ['1', '2', '3', '4']:
                logs_by_part[part].append(os.path.join(self.log_dir, filename))
        
        return logs_by_part

    def parse_ps_output(self, content):
        """Parse ps command output into structured data"""
        lines = content.strip().split('\n')
        if len(lines) < 2:  # Need at least header + 1 data row
            return []
        
        # First line is header
        header = re.split(r'\s+', lines[0].strip(), maxsplit=6)
        
        results = []
        for line in lines[1:]:
            if not line.strip():
                continue
                
            # Split by whitespace, preserving the CMD field
            parts = re.split(r'\s+', line.strip(), maxsplit=6)
            if len(parts) < len(header):
                continue
                
            # Create a dict from header and parts
            process = dict(zip(header, parts))
            results.append(process)
            
        return results

    def parse_log_file(self, path):
        """Parse a single log file and extract PS data"""
        with open(path, 'r') as f:
            content = f.read()
        
        # Filter out header lines and empty lines
        lines = [line for line in content.split('\n') if line.strip() and 'PID' not in line]
        
        if not lines:
            return []
            
        # Reconstruct proper PS output with header
        reconstructed = "PID PRI NI STAT %CPU COMMAND CMD\n" + "\n".join(lines)
        
        return self.parse_ps_output(reconstructed)

    def parse_all_logs(self):
        """Parse all log files and organize data by experiment part"""
        all_data = {}
        
        for part, log_files in self.logs_by_part.items():
            part_data = []
            
            for log_file in log_files:
                processes = self.parse_log_file(log_file)
                if processes:
                    part_data.extend(processes)
            
            all_data[part] = part_data
            
        return all_data

    def filter_experiment_processes(self, processes, experiment_pattern=r't9_|spawn'):
        """Filter processes to only include those related to our experiment"""
        return [p for p in processes if re.search(experiment_pattern, p.get('COMMAND', ''))]

    def calculate_cpu_stats(self, part):
        """Calculate CPU usage statistics for a part"""
        if part not in self.data or not self.data[part]:
            return {}
            
        processes = self.filter_experiment_processes(self.data[part])
        
        cpu_values = []
        for process in processes:
            try:
                cpu = float(process['%CPU'])
                cpu_values.append(cpu)
            except (ValueError, KeyError):
                pass
        
        if not cpu_values:
            return {}
            
        return {
            'mean': np.mean(cpu_values),
            'median': np.median(cpu_values),
            'max': np.max(cpu_values),
            'min': np.min(cpu_values),
            'std': np.std(cpu_values),
            'count': len(cpu_values)
        }

    def analyze_nice_values(self, part):
        """Analyze nice values for processes in a part"""
        if part not in self.data or not self.data[part]:
            return {}
            
        processes = self.filter_experiment_processes(self.data[part])
        
        nice_values = []
        for process in processes:
            try:
                nice = int(process['NI'])
                nice_values.append(nice)
            except (ValueError, KeyError):
                pass
        
        if not nice_values:
            return {}
            
        return Counter(nice_values)

    def analyze_process_states(self, part):
        """Analyze process states for processes in a part"""
        if part not in self.data or not self.data[part]:
            return {}
            
        processes = self.filter_experiment_processes(self.data[part])
        
        states = []
        for process in processes:
            try:
                state = process['STAT']
                states.append(state)
            except KeyError:
                pass
        
        if not states:
            return {}
            
        return Counter(states)

    def process_by_priority(self, part):
        """Group processes by priority and calculate mean CPU usage"""
        if part not in self.data or not self.data[part]:
            return {}
            
        processes = self.filter_experiment_processes(self.data[part])
        
        priority_groups = defaultdict(list)
        for process in processes:
            try:
                pri = int(process['PRI'])
                cpu = float(process['%CPU'])
                priority_groups[pri].append(cpu)
            except (ValueError, KeyError):
                pass
        
        result = {}
        for pri, cpu_values in priority_groups.items():
            result[pri] = np.mean(cpu_values) if cpu_values else 0
            
        return result

    def generate_summary_dataframe(self):
        """Generate a summary DataFrame of all analyses"""
        summary = {}
        
        for part in self.logs_by_part.keys():
            cpu_stats = self.calculate_cpu_stats(part)
            nice_values = self.analyze_nice_values(part)
            process_states = self.analyze_process_states(part)
            
            summary[f"Part {part}"] = {
                "Mean CPU %": cpu_stats.get('mean', 'N/A'),
                "Median CPU %": cpu_stats.get('median', 'N/A'),
                "CPU Std Dev": cpu_stats.get('std', 'N/A'),
                "Process Count": cpu_stats.get('count', 'N/A'),
                "Nice Values": dict(nice_values),
                "Process States": dict(process_states)
            }
            
        return pd.DataFrame(summary).T

    def plot_cpu_distribution(self, output_dir="./figures"):
        """Plot CPU distribution for each experiment part"""
        os.makedirs(output_dir, exist_ok=True)
        
        for part in self.logs_by_part.keys():
            if part not in self.data or not self.data[part]:
                continue
                
            processes = self.filter_experiment_processes(self.data[part])
            
            cpu_values = []
            for process in processes:
                try:
                    cpu = float(process['%CPU'])
                    cpu_values.append(cpu)
                except (ValueError, KeyError):
                    pass
            
            if not cpu_values:
                continue
                
            plt.figure(figsize=(10, 6))
            plt.hist(cpu_values, bins=20, alpha=0.7, color='blue')
            plt.title(f'CPU Usage Distribution - Part {part}')
            plt.xlabel('CPU Usage (%)')
            plt.ylabel('Frequency')
            plt.grid(True, alpha=0.3)
            plt.savefig(os.path.join(output_dir, f'cpu_dist_part{part}.pdf'))
            plt.close()
            
    def plot_state_distribution(self, output_dir="./figures"):
        """Plot process state distribution for each experiment part"""
        os.makedirs(output_dir, exist_ok=True)
        
        for part in self.logs_by_part.keys():
            states_counter = self.analyze_process_states(part)
            if not states_counter:
                continue
                
            states = list(states_counter.keys())
            counts = list(states_counter.values())
            
            plt.figure(figsize=(8, 6))
            plt.bar(states, counts, color='green', alpha=0.7)
            plt.title(f'Process State Distribution - Part {part}')
            plt.xlabel('Process State')
            plt.ylabel('Count')
            plt.grid(True, alpha=0.3, axis='y')
            plt.savefig(os.path.join(output_dir, f'state_dist_part{part}.pdf'))
            plt.close()

    def plot_priority_vs_cpu(self, output_dir="./figures"):
        """Plot priority vs CPU usage for each experiment part"""
        os.makedirs(output_dir, exist_ok=True)
        
        for part in self.logs_by_part.keys():
            priority_cpu = self.process_by_priority(part)
            if not priority_cpu:
                continue
                
            priorities = list(priority_cpu.keys())
            cpu_means = list(priority_cpu.values())
            
            plt.figure(figsize=(10, 6))
            plt.bar(priorities, cpu_means, color='purple', alpha=0.7)
            plt.title(f'Priority vs Mean CPU Usage - Part {part}')
            plt.xlabel('Priority (PRI)')
            plt.ylabel('Mean CPU Usage (%)')
            plt.grid(True, alpha=0.3, axis='y')
            plt.savefig(os.path.join(output_dir, f'priority_cpu_part{part}.pdf'))
            plt.close()

    def generate_latex_tables(self, output_dir="./latex"):
        """Generate LaTeX tables for the report"""
        os.makedirs(output_dir, exist_ok=True)
        
        # CPU Stats Table
        cpu_stats_data = {}
        for part in self.logs_by_part.keys():
            stats = self.calculate_cpu_stats(part)
            if stats:
                cpu_stats_data[f"Part {part}"] = stats
        
        if cpu_stats_data:
            cpu_df = pd.DataFrame(cpu_stats_data).T
            
            # Format the values
            for col in ['mean', 'median', 'max', 'min', 'std']:
                if col in cpu_df.columns:
                    cpu_df[col] = cpu_df[col].apply(lambda x: f"{x:.2f}")
            
            with open(os.path.join(output_dir, "cpu_stats_table.tex"), 'w') as f:
                f.write(cpu_df.to_latex())
        
        # Process States Table
        states_data = {}
        for part in self.logs_by_part.keys():
            states = self.analyze_process_states(part)
            if states:
                states_data[f"Part {part}"] = dict(states)
        
        if states_data:
            states_df = pd.DataFrame(states_data).fillna(0)
            states_df = states_df.applymap(lambda x: int(x) if pd.notnull(x) else 0)
            
            with open(os.path.join(output_dir, "process_states_table.tex"), 'w') as f:
                f.write(states_df.to_latex())

    def generate_report_data(self):
        """Generate all data needed for the report"""
        # Create output directories
        os.makedirs("./figures", exist_ok=True)
        os.makedirs("./latex", exist_ok=True)
        
        # Generate plots
        self.plot_cpu_distribution()
        self.plot_state_distribution()
        self.plot_priority_vs_cpu()
        
        # Generate LaTeX tables
        self.generate_latex_tables()
        
        # Generate summary dataframe and save as CSV
        summary_df = self.generate_summary_dataframe()
        summary_df.to_csv("./latex/summary_data.csv")
        
        print("Analysis complete. Data generated for LaTeX report.")
        return summary_df

if __name__ == "__main__":
    analyzer = PSAnalyzer()
    summary = analyzer.generate_report_data()
    print(summary)