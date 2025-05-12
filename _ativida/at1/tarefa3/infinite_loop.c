#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>

// Flag for graceful termination
volatile sig_atomic_t running = 1;

// Signal handler for graceful termination
void handle_signal(int sig) {
    printf("\nProcess %d received signal %d, terminating gracefully...\n", getpid(), sig);
    running = 0;
}

// Function to display memory usage
void show_memory_info() {
    char buffer[128];
    FILE* fp = fopen("/proc/self/status", "r");
    if (fp == NULL) return;
    
    size_t vm_size = 0, vm_rss = 0;
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strncmp(buffer, "VmSize:", 7) == 0) {
            sscanf(buffer, "VmSize: %zu", &vm_size);
        }
        if (strncmp(buffer, "VmRSS:", 6) == 0) {
            sscanf(buffer, "VmRSS: %zu", &vm_rss);
        }
    }
    
    fclose(fp);
    printf("Memory usage: VmSize: %zu KB, VmRSS: %zu KB\n", vm_size, vm_rss);
}

int main(int argc, char *argv[]) {
    // Register signal handlers
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);
    
    // Get process information
    pid_t pid = getpid();
    pid_t ppid = getppid();
    
    // Random seed based on PID
    srand(pid);
    
    // Determine the loop count (default or from arguments)
    unsigned long loop_count = 0;
    int sleep_time = 1;
    
    if (argc > 1) {
        sleep_time = atoi(argv[1]);
        if (sleep_time <= 0) sleep_time = 1;
    }
    
    // Print starting message with timestamp
    char timestamp[64];
    time_t now = time(NULL);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    printf("[%s] Process %d started (Parent: %d)\n", timestamp, pid, ppid);
    printf("Process will wait %d second(s) between iterations\n", sleep_time);
    show_memory_info();
    
    // Main loop
    while (running) {
        // Update loop count and periodically print status
        loop_count++;
        if (loop_count % 5 == 0) {
            now = time(NULL);
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
            printf("[%s] Process %d still running (iteration %lu)\n", 
                  timestamp, pid, loop_count);
            show_memory_info();
        }
        
        // Sleep to reduce CPU usage
        sleep(sleep_time);
    }
    
    // Print final statistics
    now = time(NULL);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    printf("[%s] Process %d completed after %lu iterations\n", timestamp, pid, loop_count);
    
    return 0;
}