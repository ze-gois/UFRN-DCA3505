#!/bin/bash

# pid_chaos_demo.sh
# This script demonstrates PID oscillation by creating and killing processes
# while monitoring the PIDs assigned to a simple program.
# 
# The script performs the following:
# 1. Creates background processes
# 2. Kills them randomly
# 3. Runs pid_info between each action to observe PID assignment

# Ensure we have all the necessary programs compiled
make all > /dev/null 2>&1

# Function to run pid_info and show output with timestamp
run_pid_info() {
    echo -e "\n---------------------------------------------"
    echo "[$iteration] Running pid_info at $(date +%H:%M:%S.%N | cut -c1-12)"
    ./pid_info
    echo "---------------------------------------------"
}

# Initialize counters
iteration=1

# Clean up any existing infinite_loop processes
echo "Cleaning up any existing background processes..."
killall -q infinite_loop 2>/dev/null

echo "===== PID CHAOS DEMONSTRATION ====="
echo "This script demonstrates how PIDs can oscillate due to system activity"
echo "Starting time: $(date)"
echo "===== MONITORING PID ASSIGNMENTS ====="

# Baseline: Run pid_info to see initial PID
run_pid_info
((iteration++))

# First round: Spawn some background processes
echo -e "\n===== ROUND 1: SPAWNING PROCESSES ====="
./spawn_processes.sh 2
sleep 1

# Run pid_info to see how PID changed
run_pid_info
((iteration++))

# Kill some of the spawned processes
echo -e "\n===== ROUND 2: KILLING PROCESSES ====="
./kill_random.sh 2
sleep 1

# Run pid_info again
run_pid_info
((iteration++))

# Second round: Spawn more processes
echo -e "\n===== ROUND 3: SPAWNING MORE PROCESSES ====="
./spawn_processes.sh 3
sleep 1

# Run pid_info
run_pid_info
((iteration++))

# Kill more processes
echo -e "\n===== ROUND 4: KILLING MORE PROCESSES ====="
./kill_random.sh 3
sleep 1

# Run pid_info
run_pid_info
((iteration++))

# Final round: Rapid spawn and kill
echo -e "\n===== ROUND 5: RAPID SPAWNING AND KILLING ====="
for i in {1..3}; do
    echo -e "\n--- Rapid cycle $i ---"
    ./spawn_processes.sh 1 > /dev/null
    sleep 0.3
    run_pid_info
    ((iteration++))
    ./kill_random.sh 1 > /dev/null
    sleep 0.3
    run_pid_info
    ((iteration++))
done

# Clean up
echo -e "\n===== CLEANING UP ====="
echo "Killing all remaining infinite_loop processes..."
killall -q infinite_loop 2>/dev/null

# Run final pid_info
echo -e "\n===== FINAL PID CHECK ====="
run_pid_info

echo -e "\n===== DEMONSTRATION COMPLETE ====="
echo "Ending time: $(date)"

# Summary
echo -e "\n===== CONCLUSION ====="
echo "This demonstration shows how PIDs are dynamically assigned by the system"
echo "and can vary unpredictably based on system activity."
echo "Each time a process is created, it gets the next available PID,"
echo "which depends on what other processes have been created or terminated."