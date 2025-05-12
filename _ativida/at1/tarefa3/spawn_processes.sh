#!/bin/bash

# Script to spawn random number of infinite_loop processes in the background

# Maximum number of processes to spawn at once (maximum 5 per your requirement)
MAX_PROCESSES=5

echo "===== SPAWNING BACKGROUND PROCESSES ====="
echo "Timestamp: $(date)"
echo "This script will start up to $MAX_PROCESSES background processes"

# Function to spawn a random number of processes
spawn_random_processes() {
    # Random number between 1 and MAX_PROCESSES
    num_to_spawn=$((1 + RANDOM % MAX_PROCESSES))
    
    echo "Spawning $num_to_spawn new background processes..."
    
    for ((i=1; i<=num_to_spawn; i++)); do
        # Start the infinite loop program in background
        ./infinite_loop &
        
        # Store the PID
        PIDS+=($!)
        
        echo "Started process $i with PID: ${PIDS[-1]}"
        
        # Small random delay between spawns (0-1 seconds)
        sleep "0.$(( RANDOM % 10 ))"
    done
    
    # Display total processes running
    echo "Total background processes now running: ${#PIDS[@]}"
    echo "PIDs: ${PIDS[*]}"
}

# Array to keep track of spawned PIDs
PIDS=()

# Spawn processes in a loop
iterations=${1:-3}  # Default to 3 iterations if not specified

for ((iter=1; iter<=iterations; iter++)); do
    echo -e "\nIteration $iter of $iterations"
    spawn_random_processes
    
    # Random wait between 1-3 seconds before next iteration
    wait_time=$((1 + RANDOM % 3))
    echo "Waiting $wait_time seconds before next iteration..."
    sleep $wait_time
done

echo -e "\n===== FINISHED SPAWNING PROCESSES ====="
echo "Timestamp: $(date)"
echo "Total processes spawned: ${#PIDS[@]}"
echo "All PIDs: ${PIDS[*]}"

# Export the PIDs so other scripts can find them
export SPAWNED_PIDS="${PIDS[*]}"
echo "PIDs exported as SPAWNED_PIDS environment variable"