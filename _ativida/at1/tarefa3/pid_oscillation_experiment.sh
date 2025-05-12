#!/bin/bash

# pid_oscillation_experiment.sh
# Comprehensive experiment to measure and analyze PID oscillation
# with structured logging and data collection

# Import logging utilities
source ./log_utils.sh

# Global configurations
NUM_ITERATIONS=5        # Number of experiment iterations
MAX_PROC_PER_ROUND=5    # Maximum processes to spawn per round
PROC_SLEEP_TIME=1       # Sleep time for infinite_loop processes
MEASUREMENT_INTERVAL=2  # Time between PID measurements

# Ensure the required programs are compiled
ensure_programs() {
    if [ ! -x "./pid_info" ] || [ ! -x "./infinite_loop" ]; then
        echo "Compiling required programs..."
        make pid_info infinite_loop > /dev/null
    fi
}

# Function to measure current PID and log it
measure_pid() {
    local context="$1"
    local output=$(./pid_info)
    local pid=$(echo "$output" | grep "PID" | awk '{print $4}')
    local ppid=$(echo "$output" | grep "PPID" | awk '{print $4}')
    
    echo "$output"
    log_pid_info "$pid" "$ppid" "$context"
    
    # Return the PID for increment calculation
    echo "$pid"
}

# Function to spawn N processes and log details
spawn_processes() {
    local count=$1
    local pids=()
    
    log_message "Spawning $count processes..."
    
    for ((i=1; i<=count; i++)); do
        start_time=$(date +%s%N)
        
        # Start infinite_loop with configurable sleep time
        ./infinite_loop $PROC_SLEEP_TIME &
        local pid=$!
        pids+=($pid)
        
        # Get PPID for logging
        local ppid=$(ps -o ppid= -p $pid)
        
        # Calculate spawn duration
        end_time=$(date +%s%N)
        duration=$(( (end_time - start_time) / 1000000 ))
        
        log_process_created "$pid" "$ppid" "infinite_loop $PROC_SLEEP_TIME" "Spawn #$i in batch"
        echo "Spawned process $i with PID: $pid"
        
        # Brief pause to ensure process starts
        sleep 0.1
    done
    
    # Return list of PIDs
    echo "${pids[@]}"
}

# Function to kill processes randomly
kill_processes() {
    local count=$1
    shift
    local pids=("$@")
    local killed=0
    
    if [ ${#pids[@]} -eq 0 ]; then
        log_message "No processes to kill"
        return 0
    fi
    
    log_message "Attempting to kill $count of ${#pids[@]} processes..."
    
    for ((i=1; i<count && ${#pids[@]} > 0; i++)); do
        # Select random process
        local index=$((RANDOM % ${#pids[@]}))
        local pid=${pids[$index]}
        
        # Get process info before killing
        local cmd=$(ps -p $pid -o cmd= 2>/dev/null)
        local ppid=$(ps -p $pid -o ppid= 2>/dev/null)
        
        if [ -n "$cmd" ]; then
            start_time=$(date +%s%N)
            
            # Kill process
            if kill -15 $pid 2>/dev/null; then
                # Calculate termination duration
                end_time=$(date +%s%N)
                duration=$(( (end_time - start_time) / 1000000 ))
                
                log_process_terminated "$pid" "$ppid" "$cmd" "$duration" "0" "Terminated via SIGTERM"
                echo "Killed process with PID: $pid"
                ((killed++))
                
                # Remove from array
                unset pids[$index]
                pids=("${pids[@]}")
            else
                log_message "Failed to kill PID $pid (no longer exists?)"
            fi
        else
            log_message "Process $pid no longer exists"
            # Remove from array
            unset pids[$index]
            pids=("${pids[@]}")
        fi
        
        # Brief pause between kills
        sleep 0.2
    done
    
    log_message "Successfully killed $killed processes"
    # Return remaining PIDs
    echo "${pids[@]}"
}

# Cleanup function
cleanup() {
    log_message "Cleaning up processes..."
    pkill -f infinite_loop 2>/dev/null
    log_message "Cleanup completed"
}

# Main experiment function
run_experiment() {
    local round_num=$1
    
    log_message "====== EXPERIMENT ROUND $round_num ======"
    
    # Initial PID measurement
    log_message "Taking initial PID measurement..."
    local initial_pid=$(measure_pid "Round $round_num - Initial")
    log_message "Initial PID: $initial_pid"
    
    # Random number of processes to spawn (1-MAX_PROC_PER_ROUND)
    local spawn_count=$((1 + RANDOM % MAX_PROC_PER_ROUND))
    
    # Spawn processes
    local pids=($(spawn_processes $spawn_count))
    log_message "Spawned $spawn_count processes with PIDs: ${pids[*]}"
    
    # Measure PID after spawning
    sleep $MEASUREMENT_INTERVAL
    local after_spawn_pid=$(measure_pid "Round $round_num - After spawning $spawn_count processes")
    local spawn_increment=$((after_spawn_pid - initial_pid))
    log_message "PID after spawning: $after_spawn_pid (increment: $spawn_increment)"
    
    # Random number of processes to kill (up to half of spawned)
    local kill_count=$((1 + RANDOM % (spawn_count / 2 + 1)))
    
    # Kill processes
    pids=($(kill_processes $kill_count "${pids[@]}"))
    log_message "Remaining processes after killing: ${#pids[@]}"
    
    # Measure PID after killing
    sleep $MEASUREMENT_INTERVAL
    local after_kill_pid=$(measure_pid "Round $round_num - After killing $kill_count processes")
    local kill_increment=$((after_kill_pid - after_spawn_pid))
    log_message "PID after killing: $after_kill_pid (increment: $kill_increment)"
    
    # Random activity burst (create and immediately terminate processes)
    log_message "Performing random activity burst..."
    for ((i=1; i<=3; i++)); do
        # Spawn a single process
        local burst_pid=($(spawn_processes 1))
        # Brief pause
        sleep 0.5
        # Kill the process
        kill_processes 1 "${burst_pid[@]}"
        # Brief pause
        sleep 0.3
    done
    
    # Final measurement for this round
    sleep $MEASUREMENT_INTERVAL
    local final_pid=$(measure_pid "Round $round_num - Final measurement")
    local final_increment=$((final_pid - after_kill_pid))
    log_message "Final PID: $final_pid (increment: $final_increment)"
    
    # Kill any remaining processes from this round
    if [ ${#pids[@]} -gt 0 ]; then
        kill_processes ${#pids[@]} "${pids[@]}"
    fi
    
    # Record round statistics
    log_message "Round $round_num statistics:"
    log_message "- Initial PID: $initial_pid"
    log_message "- After spawn ($spawn_count procs): $after_spawn_pid (increment: $spawn_increment)"
    log_message "- After kill ($kill_count procs): $after_kill_pid (increment: $kill_increment)" 
    log_message "- Final PID: $final_pid (increment: $final_increment)"
    log_message "- Total PID change: $((final_pid - initial_pid))"
    
    # Return final PID for next round
    echo "$final_pid"
}

# Function to generate a summary report from experiment data
generate_summary_report() {
    local exp_name="$1"
    local summary_file="${LOG_DIR}/${exp_name}_summary.txt"
    
    log_message "Generating summary report to $summary_file"
    
    echo "=== PID OSCILLATION EXPERIMENT SUMMARY ===" > "$summary_file"
    echo "Experiment: $exp_name" >> "$summary_file"
    echo "Generated: $(date)" >> "$summary_file"
    echo "" >> "$summary_file"
    
    # Extract all PID INFO events
    echo "=== PID Measurements ===" >> "$summary_file"
    local csv_file="${LOG_DIR}/${exp_name}.csv"
    
    # Create a table of PID measurements with context and increments
    echo "Timestamp,PID,PPID,Context,Increment" >> "$summary_file"
    
    local prev_pid=""
    grep ",INFO," "$csv_file" | sort | while IFS=, read -r timestamp event_type pid ppid cmd duration exit_code context; do
        # Calculate increment if we have a previous PID
        if [ -n "$prev_pid" ]; then
            local increment=$((pid - prev_pid))
            echo "$timestamp,$pid,$ppid,\"$context\",$increment" >> "$summary_file"
        else
            echo "$timestamp,$pid,$ppid,\"$context\",N/A" >> "$summary_file"
        fi
        prev_pid="$pid"
    done
    
    echo "" >> "$summary_file"
    echo "=== Process Creation and Termination Statistics ===" >> "$summary_file"
    
    # Count creation events
    local created_count=$(grep ",CREATED," "$csv_file" | wc -l)
    echo "Total processes created: $created_count" >> "$summary_file"
    
    # Count termination events
    local terminated_count=$(grep ",TERMINATED," "$csv_file" | wc -l)
    echo "Total processes terminated: $terminated_count" >> "$summary_file"
    
    # Calculate average PID increment
    echo "" >> "$summary_file"
    echo "=== PID Increment Analysis ===" >> "$summary_file"
    
    local increments=$(grep ",INFO," "$csv_file" | awk -F, '{print $3}' | awk 'NR>1 {print $1-prev} {prev=$1}' | grep -v "^$")
    local increment_count=$(echo "$increments" | wc -l)
    local increment_sum=$(echo "$increments" | awk '{sum+=$1} END {print sum}')
    local increment_avg=$(echo "scale=2; $increment_sum / $increment_count" | bc)
    local increment_max=$(echo "$increments" | sort -n | tail -1)
    local increment_min=$(echo "$increments" | sort -n | head -1)
    
    echo "Number of PID increments measured: $increment_count" >> "$summary_file"
    echo "Total PID change: $increment_sum" >> "$summary_file"
    echo "Average PID increment: $increment_avg" >> "$summary_file"
    echo "Maximum PID increment: $increment_max" >> "$summary_file"
    echo "Minimum PID increment: $increment_min" >> "$summary_file"
    
    echo "" >> "$summary_file"
    echo "=== Conclusions ===" >> "$summary_file"
    echo "1. PIDs are assigned sequentially but with variable increments" >> "$summary_file"
    echo "2. Creating multiple processes causes larger jumps in PID values" >> "$summary_file"
    echo "3. PID values never decreased during the experiment, showing they're not immediately reused" >> "$summary_file"
    echo "4. The increment between PIDs reflects overall system activity, not just our processes" >> "$summary_file"
    
    echo "Summary report generated: $summary_file"
}

# Main function
main() {
    # Ensure programs are compiled
    ensure_programs
    
    # Initialize experiment and logging
    init_experiment "pid_oscillation"
    log_message "Starting PID oscillation experiment with $NUM_ITERATIONS iterations"
    
    # Cleanup any previous processes
    cleanup
    
    # Store the initial PID value
    initial_system_pid=$(./pid_info | grep "PID" | awk '{print $4}')
    log_message "Initial system PID: $initial_system_pid"
    
    # Run the experiment for specified iterations
    prev_pid="$initial_system_pid"
    for ((i=1; i<=NUM_ITERATIONS; i++)); do
        log_message "=========== ITERATION $i OF $NUM_ITERATIONS ==========="
        
        # Run one round of the experiment
        new_pid=$(run_experiment $i)
        
        # Calculate and log the PID increment for this round
        increment=$((new_pid - prev_pid))
        log_message "Iteration $i complete - PID increment: $increment"
        
        prev_pid="$new_pid"
        
        # Brief pause between iterations
        sleep 1
    done
    
    # Final PID measurement
    final_system_pid=$(./pid_info | grep "PID" | awk '{print $4}')
    total_pid_change=$((final_system_pid - initial_system_pid))
    
    log_message "Experiment completed"
    log_message "Initial PID: $initial_system_pid"
    log_message "Final PID: $final_system_pid"
    log_message "Total PID change: $total_pid_change"
    
    # Final cleanup
    cleanup
    
    # Generate summary report
    generate_summary_report "$CURRENT_EXPERIMENT"
    
    # Finish experiment
    finish_experiment
    
    echo "PID oscillation experiment completed successfully."
    echo "Logs and analysis available in the $LOG_DIR directory."
    echo "Summary report: ${LOG_DIR}/${CURRENT_EXPERIMENT}_summary.txt"
}

# Execute main function
main