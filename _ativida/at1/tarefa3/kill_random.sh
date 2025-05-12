#!/bin/bash

# Script to randomly kill background processes that were spawned by spawn_processes.sh

# Function to kill a random process from the array of PIDs
kill_random_process() {
    # If no processes to kill, return
    if [ ${#PIDS[@]} -eq 0 ]; then
        echo "No processes to kill"
        return 1
    fi
    
    # Select a random index
    random_index=$((RANDOM % ${#PIDS[@]}))
    
    # Get the PID at the random index
    pid_to_kill=${PIDS[$random_index]}
    
    # Try to kill the process
    echo "Attempting to kill process with PID: $pid_to_kill"
    
    if kill -15 $pid_to_kill 2>/dev/null; then
        echo "Successfully killed process with PID: $pid_to_kill"
        
        # Remove the killed PID from the array
        unset PIDS[$random_index]
        # Re-index the array
        PIDS=("${PIDS[@]}")
        
        return 0
    else
        echo "Process with PID $pid_to_kill no longer exists"
        
        # Remove the non-existent PID from the array
        unset PIDS[$random_index]
        # Re-index the array
        PIDS=("${PIDS[@]}")
        
        return 1
    fi
}

# Initialize the PIDs array from the environment variable if provided
if [ -n "$SPAWNED_PIDS" ]; then
    # Convert the space-separated string to an array
    read -ra PIDS <<< "$SPAWNED_PIDS"
    echo "Loaded ${#PIDS[@]} PIDs from environment: ${PIDS[*]}"
else
    # Find all running infinite_loop processes
    echo "No PIDs provided from environment, finding all infinite_loop processes..."
    mapfile -t PIDS < <(pgrep -f infinite_loop)
    echo "Found ${#PIDS[@]} infinite_loop processes: ${PIDS[*]}"
fi

# Number of processes to kill
num_to_kill=${1:-$((1 + RANDOM % 3))}  # Default random between 1-3 if not specified

echo "===== KILLING RANDOM PROCESSES ====="
echo "Timestamp: $(date)"
echo "Attempting to kill $num_to_kill processes"

killed_count=0
for ((i=1; i<=num_to_kill; i++)); do
    echo -e "\nKill attempt $i of $num_to_kill"
    
    if kill_random_process; then
        ((killed_count++))
    fi
    
    # Random delay between kills (0-2 seconds)
    sleep_time="0.$((RANDOM % 20))"
    echo "Waiting $sleep_time seconds before next kill attempt..."
    sleep $sleep_time
    
    # If we've killed all processes, break
    if [ ${#PIDS[@]} -eq 0 ]; then
        echo "All processes have been killed"
        break
    fi
done

echo -e "\n===== FINISHED KILLING PROCESSES ====="
echo "Timestamp: $(date)"
echo "Successfully killed $killed_count processes"
echo "Remaining processes: ${#PIDS[@]}"
if [ ${#PIDS[@]} -gt 0 ]; then
    echo "Remaining PIDs: ${PIDS[*]}"
fi

# Export remaining PIDs
export SPAWNED_PIDS="${PIDS[*]}"
echo "Updated PIDs exported as SPAWNED_PIDS environment variable"