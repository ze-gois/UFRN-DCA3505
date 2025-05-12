#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
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

struct PsFile {
    int fd;
    char str[35];
} PsFile;

struct PsFile new_file(){
    time_t timestamp;
    time(&timestamp);
    char timestamp_str[35];
    struct tm *tm_info = localtime(&timestamp);
    strftime(timestamp_str, sizeof(timestamp_str), "./ps-%Y-%m-%d %H:%M:%S", tm_info);
    int fd = openat(AT_FDCWD, timestamp_str, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    struct PsFile result = {
            .fd = fd,
        };
    strcpy(result.str, timestamp_str);

    return result;
}

void ps(int fd, pid_t *pids, int pid_count){
    // Build a string containing all the PIDs we want to filter for
    char pid_list[1024] = "";

    for (int i = 0; i < pid_count; i++) {
        char pid_str[32];
        snprintf(pid_str, sizeof(pid_str), "%d,", pids[i]);
        strcat(pid_list, pid_str);
    }

    // Add parent process PID
    char pid_str[32];
    snprintf(pid_str, sizeof(pid_str), "%d", getpid());
    strcat(pid_list, pid_str);

    // Prepare ps command with PID filter
    char *argv[] = {
        "/usr/bin/ps",
        "-p", pid_list,
        "-o", "pid,pri,ni,stat,%cpu,cmd",
        "--sort=-%cpu",
        NULL
    };

    // Duplicate fd to stdout before execve
    if (fd != STDOUT_FILENO) {
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("dup2");
            return;
        }
    }

    execve("/usr/bin/ps", argv, environ);
    // If we get here, execve failed
    perror("execve");
}

struct PsThread {
    uint16_t n;
    pid_t pids[256];  // Array to store PIDs of busy processes
    int pid_count;    // Number of PIDs
} PsThread;

void *ps_thread(void* arg){
    struct PsThread *pt = (struct PsThread *) arg;

    for (uint64_t f = 1; f <= pt->n; f++){
        printf("Running ps iteration %lu\n", f);
        struct PsFile fth_ps_file = new_file();

        if (fth_ps_file.fd == -1) {
            perror("Failed to create file");
            continue;
        }

        pid_t fth_ps_pid = fork();

        if (fth_ps_pid == -1) {
            // Fork failed
            perror("fork");
            close(fth_ps_file.fd);
            remove(fth_ps_file.str);
            continue;
        }
        else if (fth_ps_pid == 0) {
            // Child process
            prctl(PR_SET_NAME, "ps-monitor", 0, 0, 0); // Set process name
            ps(fth_ps_file.fd, pt->pids, pt->pid_count);
            // Should not reach here unless execve fails
            exit(1);
        }
        else {
            // Parent process
            int status;
            waitpid(fth_ps_pid, &status, 0);
            close(fth_ps_file.fd);

            // Sleep between iterations
            sleep(1);
        }
    }

    return NULL;
}

int main() {
    uint8_t nof_pp_o = 1; //nof_parallel_processes_o
    uint8_t nof_pp_s = 1; //nof_parallel_processes_s
    // uint8_t nof_pp_m = 1; //nof_parallel_processes_max
    uint8_t nof_pp_m = 10; //nof_parallel_processes_max

    for (uint8_t pp = nof_pp_o; pp <= nof_pp_m; pp = pp + nof_pp_s) {
        printf("Starting test with %d busy processes\n", pp);

        pid_t parent_pid = getpid();
        pid_t child_pids[pp+1]; // +1 because we're using 1-based indexing in the loop

        // Create pp busy child processes
        for (uint8_t p = 1; p <= pp; p++) {
            pid_t child_pid = fork();

            if (child_pid == -1) {
                // Fork failed
                perror("fork");
                exit(1);
            }
            else if (child_pid == 0) {
                // Child process - just be busy
                char proc_name[32];
                snprintf(proc_name, sizeof(proc_name), "busy-proc-%d", p);
                prctl(PR_SET_NAME, proc_name, 0, 0, 0); // Set process name

                printf("Busy child process %d started with name '%s'\n", p, proc_name);
                while(1) {
                    // CPU-intensive busy loop
                }
                // This will never be reached
                exit(0);
            }
            else {
                // Parent process - record the child PID
                child_pids[p] = child_pid;
            }
        }
        // Only the parent continues here
        if (getpid() == parent_pid) {
            // Create the thread to monitor system state
            struct PsThread pt_arg = {
                .n = 5,
                .pid_count = pp,
            };

            // Store all child PIDs in the structure for filtering in ps
            for (uint8_t p = 1; p <= pp; p++) {
                pt_arg.pids[p-1] = child_pids[p];
            }

            pthread_t thread;
            printf("Starting monitoring thread for %d busy processes\n", pp);
            if (pthread_create(&thread, NULL, ps_thread, (void *)&pt_arg) != 0) {
                perror("pthread_create");
                // Kill all child processes before exiting
                for (uint8_t p = 1; p <= pp; p++) {
                    kill(child_pids[p], SIGKILL);
                }
                return 1;
            }

            // Wait for the monitoring thread to complete
            pthread_join(thread, NULL);

            printf("Monitoring completed for %d busy processes\n", pp);

            // Kill all child processes before proceeding to next test
            for (uint8_t p = 1; p <= pp; p++) {
                printf("Killing busy child process %d (PID: %d)\n", p, child_pids[p]);
                kill(child_pids[p], SIGKILL);
                waitpid(child_pids[p], NULL, 0);
            }
            printf("Test with %d busy processes completed\n\n", pp);
        } else {
            exit(0);
        }
    }

    return 0;
}
