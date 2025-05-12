#!/bin/bash

# log_utils.sh - Utility functions for experimental logging
# This script provides standardized logging functions for process experiments

# Global variables
LOG_DIR="logs"
CURRENT_EXPERIMENT=""
CURRENT_LOG_FILE=""
CSV_HEADER="timestamp,event_type,pid,ppid,command,duration_ms,exit_code,additional_info"

# Ensure the log directory exists
ensure_log_dir() {
    if [ ! -d "$LOG_DIR" ]; then
        mkdir -p "$LOG_DIR"
        echo "Created log directory: $LOG_DIR"
    fi
}

# Initialize a new experiment with a unique name
# Usage: init_experiment "experiment_name"
init_experiment() {
    ensure_log_dir
    local name="${1:-experiment}"
    local timestamp=$(date +"%Y%m%d_%H%M%S")
    CURRENT_EXPERIMENT="${name}_${timestamp}"
    CURRENT_LOG_FILE="${LOG_DIR}/${CURRENT_EXPERIMENT}.log"
    local csv_file="${LOG_DIR}/${CURRENT_EXPERIMENT}.csv"
    
    # Create the log file with a header
    echo "=== ${CURRENT_EXPERIMENT} ===" > "$CURRENT_LOG_FILE"
    echo "Start time: $(date)" >> "$CURRENT_LOG_FILE"
    echo "System: $(uname -a)" >> "$CURRENT_LOG_FILE"
    echo "" >> "$CURRENT_LOG_FILE"
    
    # Create CSV file with header
    echo "$CSV_HEADER" > "$csv_file"
    
    echo "Initialized experiment: $CURRENT_EXPERIMENT"
    echo "Log file: $CURRENT_LOG_FILE"
    echo "CSV file: $csv_file"
    
    return 0
}

# Log a message to the current experiment log file
# Usage: log_message "message"
log_message() {
    if [ -z "$CURRENT_LOG_FILE" ]; then
        echo "ERROR: No active experiment. Call init_experiment first."
        return 1
    fi
    
    local timestamp=$(date +"%Y-%m-%d %H:%M:%S.%N" | cut -c1-23)
    echo "[${timestamp}] $1" >> "$CURRENT_LOG_FILE"
    
    return 0
}

# Log a process event in CSV format
# Usage: log_process_event "event_type" pid ppid "command" duration_ms exit_code "additional_info"
log_process_event() {
    if [ -z "$CURRENT_EXPERIMENT" ]; then
        echo "ERROR: No active experiment. Call init_experiment first."
        return 1
    fi
    
    local event_type="$1"
    local pid="$2"
    local ppid="$3"
    local command="$4"
    local duration="$5"
    local exit_code="$6"
    local additional_info="$7"
    
    local timestamp=$(date +"%Y-%m-%d %H:%M:%S.%N" | cut -c1-23)
    local csv_file="${LOG_DIR}/${CURRENT_EXPERIMENT}.csv"
    
    # Escape commas in strings
    command=$(echo "$command" | sed 's/,/;/g')
    additional_info=$(echo "$additional_info" | sed 's/,/;/g')
    
    # Log to CSV
    echo "${timestamp},${event_type},${pid},${ppid},${command},${duration},${exit_code},${additional_info}" >> "$csv_file"
    
    # Also log to text log file
    log_message "EVENT: $event_type | PID: $pid | PPID: $ppid | CMD: $command | Additional: $additional_info"
    
    return 0
}

# Log process creation
# Usage: log_process_created pid ppid "command" "additional_info"
log_process_created() {
    log_process_event "CREATED" "$1" "$2" "$3" "0" "0" "$4"
}

# Log process termination
# Usage: log_process_terminated pid ppid "command" duration_ms exit_code "additional_info"
log_process_terminated() {
    log_process_event "TERMINATED" "$1" "$2" "$3" "$4" "$5" "$6"
}

# Log PID information
# Usage: log_pid_info pid ppid "context_description"
log_pid_info() {
    log_process_event "INFO" "$1" "$2" "pid_info" "0" "0" "$3"
}

# Finish an experiment with summary information
# Usage: finish_experiment
finish_experiment() {
    if [ -z "$CURRENT_LOG_FILE" ]; then
        echo "ERROR: No active experiment. Call init_experiment first."
        return 1
    fi
    
    echo "" >> "$CURRENT_LOG_FILE"
    echo "End time: $(date)" >> "$CURRENT_LOG_FILE"
    echo "=== End of ${CURRENT_EXPERIMENT} ===" >> "$CURRENT_LOG_FILE"
    
    # Generate summary if CSV file exists
    local csv_file="${LOG_DIR}/${CURRENT_EXPERIMENT}.csv"
    if [ -f "$csv_file" ]; then
        local total_events=$(wc -l < "$csv_file")
        local created_events=$(grep ",CREATED," "$csv_file" | wc -l)
        local terminated_events=$(grep ",TERMINATED," "$csv_file" | wc -l)
        local info_events=$(grep ",INFO," "$csv_file" | wc -l)
        
        echo "" >> "$CURRENT_LOG_FILE"
        echo "=== SUMMARY ===" >> "$CURRENT_LOG_FILE"
        echo "Total events: $((total_events-1)) (excluding header)" >> "$CURRENT_LOG_FILE"
        echo "Process creations: $created_events" >> "$CURRENT_LOG_FILE"
        echo "Process terminations: $terminated_events" >> "$CURRENT_LOG_FILE"
        echo "PID info events: $info_events" >> "$CURRENT_LOG_FILE"
    fi
    
    echo "Experiment $CURRENT_EXPERIMENT completed"
    
    # Reset experiment variables
    CURRENT_EXPERIMENT=""
    CURRENT_LOG_FILE=""
    
    return 0
}

# Generate a statistical analysis of the experiment
# Usage: analyze_experiment "experiment_name_timestamp"
analyze_experiment() {
    local exp_name="$1"
    if [ -z "$exp_name" ]; then
        echo "ERROR: Experiment name required."
        return 1
    fi
    
    local csv_file="${LOG_DIR}/${exp_name}.csv"
    if [ ! -f "$csv_file" ]; then
        echo "ERROR: CSV file not found: $csv_file"
        return 1
    fi
    
    local analysis_file="${LOG_DIR}/${exp_name}_analysis.txt"
    
    echo "=== ANALYSIS OF EXPERIMENT: $exp_name ===" > "$analysis_file"
    echo "Generated at: $(date)" >> "$analysis_file"
    echo "" >> "$analysis_file"
    
    # PID assignment analysis
    echo "=== PID ASSIGNMENT ANALYSIS ===" >> "$analysis_file"
    echo "Extracting PIDs from INFO events in chronological order:" >> "$analysis_file"
    
    grep ",INFO," "$csv_file" | awk -F, '{print $1 " " $3 " " $4 " " $8}' | \
    while read -r timestamp pid ppid desc; do
        echo "Time: $timestamp | PID: $pid | PPID: $ppid | Context: $desc" >> "$analysis_file"
    done
    
    echo "" >> "$analysis_file"
    echo "=== PID INCREMENT ANALYSIS ===" >> "$analysis_file"
    
    # Extract PIDs in order and calculate differences
    grep ",INFO," "$csv_file" | awk -F, '{print $3}' | awk 'NR>1 {print $1, $1-prev} {prev=$1}' | \
    while read -r pid diff; do
        if [ ! -z "$diff" ]; then
            echo "PID: $pid | Increment from previous: $diff" >> "$analysis_file"
        fi
    done
    
    echo "" >> "$analysis_file"
    echo "=== PROCESS LIFECYCLE ANALYSIS ===" >> "$analysis_file"
    
    # Match CREATED and TERMINATED events to calculate lifespans
    echo "Process lifespans (where both creation and termination were logged):" >> "$analysis_file"
    
    grep ",CREATED," "$csv_file" | awk -F, '{print $3 "," $1}' | sort | \
    while IFS=, read -r pid creation_time; do
        termination_line=$(grep ",TERMINATED,$pid," "$csv_file")
        if [ ! -z "$termination_line" ]; then
            termination_time=$(echo "$termination_line" | awk -F, '{print $1}')
            duration=$(echo "$termination_line" | awk -F, '{print $6}')
            cmd=$(echo "$termination_line" | awk -F, '{print $5}')
            echo "PID: $pid | Command: $cmd | Created: $creation_time | Terminated: $termination_time | Duration: $duration ms" >> "$analysis_file"
        fi
    done
    
    echo "" >> "$analysis_file"
    echo "=== END OF ANALYSIS ===" >> "$analysis_file"
    
    echo "Analysis complete. Results saved to: $analysis_file"
    return 0
}

# If this script is run directly, show usage information
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "This is a utility script providing logging functions for process experiments."
    echo "Source this script in other scripts to use its functions."
    echo ""
    echo "Main functions:"
    echo "  init_experiment \"experiment_name\"   - Start a new logging session"
    echo "  log_message \"message\"              - Log a text message"
    echo "  log_process_created pid ppid cmd info - Log process creation"
    echo "  log_process_terminated pid ppid cmd duration exit_code info - Log process end"
    echo "  log_pid_info pid ppid \"context\"     - Log PID information"
    echo "  finish_experiment                    - End experiment and generate summary"
    echo "  analyze_experiment \"experiment_name\" - Generate analysis report"
fi