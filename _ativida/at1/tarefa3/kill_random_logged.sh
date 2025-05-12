#!/bin/bash

# kill_random_logged.sh
# Script to randomly kill background processes with detailed logging

# Import logging utilities
source ./log_utils.sh

# Initialize the experiment
init_experiment "process_termination"

log_message "===== KILLING RANDOM PROCESSES ====="

# Function to kill a random process from the array of PIDs
kill_random_process() {
    # If no processes to kill, return
    if [ ${#PIDS[@]} -eq 0 ]; then
        log_message "No processes to kill"
        return 1
    fi
    
    # Select a random index
    random_index=$((RANDOM % ${#PIDS[@]}))
    
    # Get the PID at the random index
    pid_to_kill=${PIDS[$random_index]}
    
    # Try to get information about the process before killing it
    process_info=$(ps -p $pid_to_kill -o cmd=,ppid= 2>/dev/null)
    
    if [ -n "$process_info" ]; then
        cmd=$(echo "$process_info" | awk '{print $1}')
        ppid=$(echo "$process_info" | awk '{print $NF}')
        
        # Record start time for precise timing
        start_time=$(date +%s%N)
        
        log_message "Attempting to kill process with PID: $pid_to_kill (Command: $cmd, PPID: $ppid)"
        
        # Try to kill the process
        if kill -15 $pid_to_kill 2>/dev/null; then
            # Calculate duration in milliseconds
            end_time=$(date +%s%N)
            duration=$(( (end_time - start_time) / 1000000 ))
            
            # Log the process termination
            log_process_terminated "$pid_to_kill" "$ppid" "$cmd" "$duration" "0" "Terminated via SIGTERM"
            
            log_message "Successfully killed process with PID: $pid_to_kill"
            
            # Remove the killed PID from the array
            unset PIDS[$random_index]
            # Re-index the array
            PIDS=("${PIDS[@]}")
            
            return 0
        else
            log_message "Failed to kill process with PID $pid_to_kill"
            
            # Calculate duration in milliseconds
            end_time=$(date +%s%N)
            duration=$(( (end_time - start_time) / 1000000 ))
            
            # Log the failed attempt
            log_process_event "KILL_FAILED" "$pid_to_kill" "$ppid" "$cmd" "$duration" "1" "Failed to terminate"
            
            # Remove the non-existent PID from the array
            unset PIDS[$random_index]
            # Re-index the array
            PIDS=("${PIDS[@]}")
            
            return 1
        fi
    else
        log_message "Process with PID $pid_to_kill no longer exists"
        
        # Log the non-existent process event
        log_process_event "NONEXISTENT" "$pid_to_kill" "?" "unknown" "0" "1" "Process no longer exists"
        
        # Remove the non-existent PID from the array
        unset PIDS[$random_index]
        # Re-index the array
        PIDS=("${PIDS[@]}")
        
        return 1
    fi
}

# Update function to log current PID info
log_current_pid_info() {
    local context="$1"
    local current_pid=$(bash -c 'echo $$')
    local current_ppid=$(bash -c 'echo $PPID')
    
    log_pid_info "$current_pid" "$current_ppid" "$context"
    
    ./pid_info
    
    # Get the output from pid_info for logging
    pid_output=$(./pid_info)
    log_message "PID info: $pid_output"
}

# Initialize the PIDs array from the environment variable if provided
if [ -n "$SPAWNED_PIDS" ]; then
    # Convert the space-separated string to an array
    read -ra PIDS <<< "$SPAWNED_PIDS"
    log_message "Loaded ${#PIDS[@]} PIDs from environment: ${PIDS[*]}"
else
    # Find all running infinite_loop processes
    log_message "No PIDs provided from environment, finding all infinite_loop processes..."
    mapfile -t PIDS < <(pgrep -f infinite_loop)
    log_message "Found ${#PIDS[@]} infinite_loop processes: ${PIDS[*]}"
fi

# Log the state before killing anything
log_current_pid_info "Before killing any processes"

# Number of processes to kill
num_to_kill=${1:-$((1 + RANDOM % 3))}  # Default random between 1-3 if not specified

log_message "Attempting to kill $num_to_kill processes"

killed_count=0
for ((i=1; i<=num_to_kill; i++)); do
    log_message "Kill attempt $i of $num_to_kill"
    
    if kill_random_process; then
        ((killed_count++))
    fi
    
    # Log PID info after each kill
    log_current_pid_info "After kill attempt $i"
    
    # Random delay between kills (0-2 seconds)
    sleep_time="0.$((RANDOM % 20))"
    log_message "Waiting $sleep_time seconds before next kill attempt..."
    sleep $sleep_time
    
    # If we've killed all processes, break
    if [ ${#PIDS[@]} -eq 0 ]; then
        log_message "All processes have been killed"
        break
    fi
done

log_message "===== FINISHED KILLING PROCESSES ====="
log_message "Successfully killed $killed_count processes"
log_message "Remaining processes: ${#PIDS[@]}"
if [ ${#PIDS[@]} -gt 0 ]; then
    log_message "Remaining PIDs: ${PIDS[*]}"
fi

# Export remaining PIDs
export SPAWNED_PIDS="${PIDS[*]}"
log_message "Updated PIDs exported as SPAWNED_PIDS environment variable"

# Generate an analysis of the experiment
analyze_experiment "$CURRENT_EXPERIMENT"

# Finish the experiment
finish_experiment

echo "Process termination complete. Logs saved to $LOG_DIR directory."