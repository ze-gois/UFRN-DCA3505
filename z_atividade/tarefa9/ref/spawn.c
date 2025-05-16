#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/prctl.h>

extern char **environ;

// Structure for experiment settings
struct Experiment {
    uint8_t num_cpu_cores;      // Number of CPU cores (N)
    uint8_t num_processes;      // Number of processes to spawn
    int monitoring_iterations;  // Number of monitoring iterations
    int monitoring_interval;    // Interval between monitoring in seconds
    int increase_priority_pid;  // PID of process to increase priority (-1 if none)
    int blocking_input_pid;     // PID of blocking input process (-1 if none)
    char experiment_name[64];   // Name of the experiment for logging
};

// PID list for monitoring
pid_t child_pids[256];
int child_pid_count = 0;

// Directory for logs
#define LOG_DIR "./log"

// Function to get the number of CPU cores
int get_cpu_cores() {
    FILE *fp;
    char result[16];
    int cores = 1; // Default to 1 if we can't determine
    
    fp = popen("nproc", "r");
    if (fp == NULL) {
        perror("Failed to run nproc command");
        return cores;
    }

    if (fgets(result, sizeof(result), fp) != NULL) {
        cores = atoi(result);
    }
    
    pclose(fp);
    return cores;
}

// Create a new log file
struct LogFile {
    int fd;
    char filename[128];
};

struct LogFile create_log_file(const char *prefix, const char *experiment_name) {
    struct LogFile result = {.fd = -1};
    
    // Create logs directory if it doesn't exist
    mkdir(LOG_DIR, 0755);
    
    time_t timestamp;
    time(&timestamp);
    struct tm *tm_info = localtime(&timestamp);
    
    // Create a base filename
    char base_filename[64];
    snprintf(base_filename, sizeof(base_filename), 
             "%s/%s-%s", 
             LOG_DIR, prefix, experiment_name);
    
    // Format the timestamp into the filename
    char timestamp_str[64];
    strftime(timestamp_str, sizeof(timestamp_str), 
             "-%Y-%m-%d-%H-%M-%S.log", tm_info);
             
    // Combine base name and timestamp
    snprintf(result.filename, sizeof(result.filename),
             "%s%s", base_filename, timestamp_str);
    
    result.fd = open(result.filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    
    if (result.fd == -1) {
        perror("Failed to create log file");
    }
    
    return result;
}

// Execute ps command and capture output
void run_ps(int fd) {
    // Build a string containing all the PIDs we want to filter for
    char pid_list[1024] = "";
    
    for (int i = 0; i < child_pid_count; i++) {
        char pid_str[32];
        snprintf(pid_str, sizeof(pid_str), "%d,", child_pids[i]);
        strcat(pid_list, pid_str);
    }
    
    // Add parent process PID
    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d", getpid());
    strcat(pid_list, pid_str);
    
    // Create a command string directly
    char command[2048];
    snprintf(command, sizeof(command), 
             "ps -p %s -o pid,pri,nice,state,pcpu,cputime,comm --no-headers", 
             pid_list);
    
    // Duplicate fd to stdout
    if (fd != STDOUT_FILENO) {
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("dup2");
            return;
        }
        
        // Also redirect stderr to the same file
        if (dup2(fd, STDERR_FILENO) == -1) {
            perror("dup2 for stderr");
            return;
        }
    }
    
    // Execute using shell
    execl("/bin/sh", "sh", "-c", command, NULL);
    // If we get here, execl failed
    perror("execl sh failed");
    exit(1);
}

// Function to increase process priority
void increase_process_priority(pid_t pid) {
    pid_t renice_pid = fork();
    
    if (renice_pid == 0) {
        // Child process that will run renice
        char pid_str[32];
        snprintf(pid_str, sizeof(pid_str), "%d", pid);
        
        execl("/usr/bin/renice", "renice", "-n", "-10", "-p", pid_str, NULL);
        perror("execl renice failed");
        exit(1);
    } else if (renice_pid > 0) {
        // Parent process
        int status;
        waitpid(renice_pid, &status, 0);
        printf("Increased priority of process %d\n", pid);
    } else {
        perror("Fork for renice failed");
    }
}

// Thread function for monitoring processes
void *monitoring_thread(void *arg) {
    struct Experiment *exp = (struct Experiment *)arg;
    
    printf("Starting monitoring for %s with %d iterations, interval %d seconds\n", 
           exp->experiment_name, exp->monitoring_iterations, exp->monitoring_interval);
    
    // If we need to increase priority for a process, do it now
    if (exp->increase_priority_pid > 0) {
        sleep(2); // Wait a bit to let processes stabilize
        printf("Increasing priority of process %d\n", exp->increase_priority_pid);
        increase_process_priority(exp->increase_priority_pid);
    }
    
    for (int i = 1; i <= exp->monitoring_iterations; i++) {
        printf("Running ps iteration %d for %s\n", i, exp->experiment_name);
        
        struct LogFile log_file = create_log_file("ps", exp->experiment_name);
        
        if (log_file.fd == -1) {
            continue;
        }
        
        // Log iteration header
        char header[256];
        snprintf(header, sizeof(header), 
                 "=== Monitoring Iteration %d - %s ===\n", 
                 i, exp->experiment_name);
        write(log_file.fd, header, strlen(header));
        
        // Fork for ps command
        pid_t ps_pid = fork();
        
        if (ps_pid == 0) {
            // Child process
            prctl(PR_SET_NAME, "ps-monitor", 0, 0, 0);
            run_ps(log_file.fd);
            // Should not return
            exit(1);
        } else if (ps_pid > 0) {
            // Parent process
            int status;
            waitpid(ps_pid, &status, 0);
            close(log_file.fd);
            
            // Sleep between iterations
            sleep(exp->monitoring_interval);
        } else {
            perror("Fork for ps failed");
            close(log_file.fd);
        }
    }
    
    return NULL;
}

// Function to create a busy process
pid_t create_busy_process(int process_num) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process - just be busy
        char proc_name[32];
        snprintf(proc_name, sizeof(proc_name), "busy-proc-%d", process_num);
        prctl(PR_SET_NAME, proc_name, 0, 0, 0);
        
        printf("Busy process %d started (PID: %d)\n", process_num, getpid());
        
        // Infinite CPU-intensive loop
        while(1) {
            // Waste CPU cycles
            for (volatile long long i = 0; i < 10000000; i++);
        }
        
        // Should never reach here
        exit(0);
    }
    
    return pid;
}

// Function to create a blocking input process
pid_t create_blocking_process() {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process - waits for input
        prctl(PR_SET_NAME, "blocking-proc", 0, 0, 0);
        printf("Blocking input process started (PID: %d)\n", getpid());
        printf("Type anything to interact with the blocking process...\n");
        
        // Infinite loop waiting for input
        char buffer[256];
        while(1) {
            // This will block until input is available
            if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
                printf("Blocking process received input: %s", buffer);
            }
        }
        
        // Should never reach here
        exit(0);
    }
    
    return pid;
}

// Function to run an experiment
void run_experiment(struct Experiment *exp) {
    printf("\n==== Starting experiment: %s ====\n", exp->experiment_name);
    printf("- Number of CPU cores: %d\n", exp->num_cpu_cores);
    printf("- Number of processes: %d\n", exp->num_processes);
    
    // Reset PID list
    child_pid_count = 0;
    
    // Create busy processes
    int blocking_process_index = -1;
    
    for (int i = 1; i <= exp->num_processes; i++) {
        // The last process in part 4 should be a blocking process
        if (exp->blocking_input_pid > 0 && i == exp->num_processes) {
            child_pids[child_pid_count] = create_blocking_process();
            blocking_process_index = child_pid_count;
        } else {
            child_pids[child_pid_count] = create_busy_process(i);
        }
        
        if (child_pids[child_pid_count] <= 0) {
            perror("Failed to create child process");
            continue;
        }
        
        child_pid_count++;
    }
    
    // If we need to increase priority for a process, note the PID
    if (exp->increase_priority_pid > 0) {
        exp->increase_priority_pid = child_pids[exp->increase_priority_pid - 1];
    }
    
    // If we need a blocking process PID, note it
    if (blocking_process_index >= 0) {
        exp->blocking_input_pid = child_pids[blocking_process_index];
    }
    
    // Create and start monitoring thread
    pthread_t monitor_thread;
    if (pthread_create(&monitor_thread, NULL, monitoring_thread, exp) != 0) {
        perror("Failed to create monitoring thread");
        // Kill all child processes before exiting
        for (int i = 0; i < child_pid_count; i++) {
            kill(child_pids[i], SIGKILL);
        }
        return;
    }
    
    // Wait for monitoring to complete
    pthread_join(monitor_thread, NULL);
    
    printf("Monitoring completed for experiment: %s\n", exp->experiment_name);
    
    // Kill all child processes
    for (int i = 0; i < child_pid_count; i++) {
        printf("Killing child process PID: %d\n", child_pids[i]);
        kill(child_pids[i], SIGKILL);
        waitpid(child_pids[i], NULL, 0);
    }
    
    printf("==== Experiment completed: %s ====\n", exp->experiment_name);
    sleep(1); // Brief pause between experiments
}

int main() {
    // Get number of CPU cores
    int cpu_cores = get_cpu_cores();
    printf("Detected %d CPU cores\n", cpu_cores);
    
    // Create log directory if it doesn't exist
    mkdir(LOG_DIR, 0755);
    
    // Part 1: N processes (equal to CPU cores)
    struct Experiment part1 = {
        .num_cpu_cores = cpu_cores,
        .num_processes = cpu_cores,
        .monitoring_iterations = 5,
        .monitoring_interval = 2,
        .increase_priority_pid = -1,
        .blocking_input_pid = -1,
        .experiment_name = "Part1-N_Processes"
    };
    run_experiment(&part1);
    
    // Part 2: N+1 processes (more than CPU cores)
    struct Experiment part2 = {
        .num_cpu_cores = cpu_cores,
        .num_processes = cpu_cores + 1,
        .monitoring_iterations = 5,
        .monitoring_interval = 2,
        .increase_priority_pid = -1,
        .blocking_input_pid = -1,
        .experiment_name = "Part2-N_Plus_1_Processes"
    };
    run_experiment(&part2);
    
    // Part 3: N processes with one having high priority
    struct Experiment part3 = {
        .num_cpu_cores = cpu_cores,
        .num_processes = cpu_cores,
        .monitoring_iterations = 5,
        .monitoring_interval = 2,
        .increase_priority_pid = 1, // Increase priority of first process
        .blocking_input_pid = -1,
        .experiment_name = "Part3-Priority_Effect"
    };
    run_experiment(&part3);
    
    // Part 4: N CPU-intensive processes + 1 blocking process
    struct Experiment part4 = {
        .num_cpu_cores = cpu_cores,
        .num_processes = cpu_cores + 1,
        .monitoring_iterations = 5,
        .monitoring_interval = 2,
        .increase_priority_pid = -1,
        .blocking_input_pid = 1, // The last process will be blocking
        .experiment_name = "Part4-Blocked_Process"
    };
    run_experiment(&part4);
    
    printf("\nAll experiments completed.\n");
    printf("Log files are in the %s directory.\n", LOG_DIR);
    
    return 0;
}