#include <stdint.h>
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

void ps_thread(void){
    for (uint64_t f = 1; f <= 1000; f++){
        printf("Running ps iteration %lu\n", f);
        struct PsFile ps_file = new_file();
        
        if (ps_file.fd == -1) {
            perror("Failed to create file");
            continue;
        }
        
        pid_t child_pid = fork();
        
        if (child_pid == -1) {
            // Fork failed
            perror("fork");
            close(ps_file.fd);
            remove(ps_file.str);
            continue;
        } 
        else if (child_pid == 0) {
            // Child process
            ps(ps_file.fd);
            // Should not reach here unless execve fails
            exit(1);
        } 
        else {
            // Parent process
            int status;
            waitpid(child_pid, &status, 0);
            close(ps_file.fd);
            
            // Sleep between iterations
            sleep(1);
        }
    }
}

int main() {
    pthread_t thread;
    if (pthread_create(&thread, NULL, (void *(*)(void *))ps_thread, NULL) != 0) {
        perror("pthread_create");
        return 1;
    }

    // Wait for the thread to finish
    pthread_join(thread, NULL);

    // pthread_create(pthread_t *restrict newthread, const pthread_attr_t *restrict attr, void *(*start_routine)(void *), void *restrict arg)
    // pid_t
    // fork();
    return 0;
}
