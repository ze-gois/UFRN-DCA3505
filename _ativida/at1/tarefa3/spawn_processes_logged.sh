#!/bin/bash

# spawn_processes_logged.sh
# Spawns multiple infinite_loop processes with comprehensive logging

# Import logging utilities
source ./log_utils.sh

# Maximum number of processes to spawn at once (maximum 5 per requirement)
MAX_PROCESSES=5

# Initialize the experiment
init_experiment "process_spawn"

log_message "===== SPAWNING BACKGROUND PROCESSES ====="
log_message "This script will start up to $MAX_PROCESSES background processes"

# Array to keep track of spawned PIDs
PIDS=()

# Function to spawn a random number of processes
spawn_random_processes() {
    # Random number between 1 and MAX_PROCESSES
    num_to_spawn=$((1 + RANDOM % MAX_PROCESSES))
    
    log_message "Spawning $num_to_spawn new background processes..."
    
    for ((i=1; i<=num_to_spawn; i++)); do
        # Record start time in nanoseconds for precise timing
        start_time=$(date +%s%N)
        
        # Start the infinite loop program in background
        ./infinite_loop &
        
        # Store the PID
        pid=$!
        PIDS+=($pid)
        
        # Get parent PID
        ppid=$(ps -o ppid= -p $pid)
        
        # Calculate duration in milliseconds
        end_time=$(date +%s%N)
        duration=$(( (end_time - start_time) / 1000000 ))
        
        # Log the process creation
        log_process_created "$pid" "$ppid" "infinite_loop" "Spawn #$i in batch, will sleep between iterations"
        
        echo "Started process $i with PID: $pid"
        
        # Small random delay between spawns (0-1 seconds)
        delay="0.$(( RANDOM % 10 ))"
        log_message "Waiting $delay seconds before next spawn..."
        sleep "$delay"
    done
    
    # Display total processes running
    log_message "Total background processes now running: ${#PIDS[@]}"
    log_message "PIDs: ${PIDS[*]}"
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

# Spawn processes in a loop
iterations=${1:-3}  # Default to 3 iterations if not specified

# Log initial PID information
log_current_pid_info "Before spawning any processes"

for ((iter=1; iter<=iterations; iter++)); do
    log_message "Iteration $iter of $iterations"
    spawn_random_processes
    
    # Log PID info after spawning
    log_current_pid_info "After spawn iteration $iter"
    
    # Random wait between 1-3 seconds before next iteration
    wait_time=$((1 + RANDOM % 3))
    log_message "Waiting $wait_time seconds before next iteration..."
    sleep $wait_time
done

log_message "===== FINISHED SPAWNING PROCESSES ====="
log_message "Total processes spawned: ${#PIDS[@]}"
log_message "All PIDs: ${PIDS[*]}"

# Export the PIDs so other scripts can find them
export SPAWNED_PIDS="${PIDS[*]}"
log_message "PIDs exported as SPAWNED_PIDS environment variable"

# Generate an analysis of the experiment
analyze_experiment "$CURRENT_EXPERIMENT"

# Finish the experiment
finish_experiment

echo "Process spawning complete. Logs saved to $LOG_DIR directory."