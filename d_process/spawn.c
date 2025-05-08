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

void ps(int fd){
    char *argv[] = {"/usr/bin/ps", "-eo", "pid,pri,ni,stat,%cpu,cmd", "--sort=-%cpu", NULL};

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
    uint16_t p;
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
            ps(fth_ps_file.fd);
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
    uint8_t nof_pp_m = 1; //nof_parallel_processes_max



    for (uint8_t pp = nof_pp_o; pp <= nof_pp_m; pp = pp + nof_pp_s){
        pid_t pa_pid[pp] = {};
        for (uint8_t p = 1; p <= pp; p++){
            pa_pid[p] = fork();
        }

        if (pp == 1 && getpid() != 0){
            struct PsThread pt_arg =  {
                .n = 10,
                .p = pp,
            };

            pthread_t thread;
            if (pthread_create(&thread, NULL, ps_thread, (void *)&pt_arg) != 0) {
                perror("pthread_create");
                return 1;
            }
            pthread_join(thread, NULL);

            for (uint8_t p = 1; p <= pp; p++){
                kill(pa_pid[p],9);
            }
        } else {
            while(1){}
        }
    }

    return 0;
}
