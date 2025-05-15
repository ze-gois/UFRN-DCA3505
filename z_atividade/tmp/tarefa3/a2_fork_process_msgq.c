#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

struct Experiment {
    size_t repetitions;
    char description[30];
    unsigned int sleep_parent;
    unsigned int sleep_child;
    unsigned int sleep_post;
    bool do_parent_wait_child;
};

enum FORK_RESULT {
    FORK_FAIL = -1,
    FORK_CHILD = 0,
    FORK_PARENT = 1,
};

enum FORK_RESULT check_fork(pid_t pid){
    if (pid < 0) return FORK_FAIL;
    if (pid > 0) return FORK_PARENT;
    return FORK_CHILD;
}

void print_process(char const d[]) {
    printf("[%s]: (%d,%d)\n", d, getpid(), getppid());
}

void sleep_process(char const d[], unsigned int duration){
    printf("[%s]: (%d,%d) sleep for %d seconds\n", d, getpid(), getppid(), duration);
    sleep(duration);
}

// Message structure
struct msg_buffer {
    long msg_type;  // Message type, must be > 0
    char msg_text[20]; // Message content
};

// Message queue ID
int msgq_id = -1;

// Initialize message queue
bool init_lock() {
    key_t key;
    
    // Generate a key using the current directory and a predefined project ID
    if ((key = ftok(".", 'A')) == -1) {
        perror("ftok");
        return false;
    }
    
    // Create or get the message queue
    if ((msgq_id = msgget(key, 0666 | IPC_CREAT)) == -1) {
        perror("msgget");
        return false;
    }
    
    return true;
}

// Send a message to the queue
bool send_message(long msg_type, const char *text) {
    struct msg_buffer message;
    
    message.msg_type = msg_type;
    strncpy(message.msg_text, text, sizeof(message.msg_text) - 1);
    message.msg_text[sizeof(message.msg_text) - 1] = '\0';
    
    if (msgsnd(msgq_id, &message, sizeof(message.msg_text), 0) == -1) {
        perror("msgsnd");
        return false;
    }
    
    return true;
}

// Receive a message from the queue
bool receive_message(long msg_type, char *text, size_t text_size) {
    struct msg_buffer message;
    
    if (msgrcv(msgq_id, &message, sizeof(message.msg_text), msg_type, 0) == -1) {
        perror("msgrcv");
        return false;
    }
    
    strncpy(text, message.msg_text, text_size - 1);
    text[text_size - 1] = '\0';
    
    return true;
}

// Clean up message queue
void cleanup_msgq() {
    if (msgq_id != -1) {
        // Only the parent should remove the queue at the end
        if (msgctl(msgq_id, IPC_RMID, NULL) == -1) {
            perror("msgctl");
        }
        msgq_id = -1;
    }
}

void experiment(struct Experiment *e) {
    // Initialize the synchronization mechanism
    if (!init_lock()) {
        fprintf(stderr, "Failed to initialize message queue\n");
        return;
    }

    pid_t child_pid = fork();
    char fc[2];
    char msg_buf[20];

    switch (check_fork(child_pid)) {
        case FORK_FAIL:
            perror("fork");
            cleanup_msgq();
            break;

        case FORK_CHILD:
            strcpy(fc, "C");
            print_process(fc);
            sleep_process(fc, e->sleep_child);
            
            // Child notifies parent it's done with critical section
            printf("[C] Sending message to parent\n");
            send_message(1, "CHILD_DONE");
            break;

        case FORK_PARENT:
            strcpy(fc, "P");
            print_process(fc);
            sleep_process(fc, e->sleep_parent);
            
            // Parent waits for child to finish critical section
            printf("[P] Waiting for message from child\n");
            receive_message(1, msg_buf, sizeof(msg_buf));
            printf("[P] Received message from child: %s\n", msg_buf);
            break;
    }

    sleep_process(fc, e->sleep_post);
    print_process(fc);

    if (e->do_parent_wait_child && check_fork(child_pid) == FORK_PARENT) {
        int status;
        wait(&status);
        printf("[P] Child has exited with status: %d\n", WEXITSTATUS(status));
    }

    printf("[%s] End of experiment\n", fc);
    
    if (check_fork(child_pid) == FORK_CHILD) {
        // Child exits without cleaning up the queue
        exit(0);
    } else if (check_fork(child_pid) == FORK_PARENT && !e->do_parent_wait_child) {
        // Only clean up if we're not waiting for the child
        // (otherwise we'll clean up after wait())
    }
}

int main() {
    struct Experiment experiments[] = {
        {2, "Message Queue Sync", 1, 3, 1, true},
        {1, "No Wait Example", 1, 3, 1, false},
    };

    size_t nof_experiments = sizeof(experiments)/sizeof(struct Experiment);

    print_process("M");

    for (size_t e = 0; e < nof_experiments; e++) {
        printf("%s\n", experiments[e].description);
        printf("------\n");
        for (size_t r = 0; r < experiments[e].repetitions; r++) {
            printf("\t----%zu\n", r);
            experiment(&experiments[e]);
            printf("\t----%zu\n", r);
        }
        printf("------\n");
    }
    
    // Clean up message queue at the end
    cleanup_msgq();
    
    return 0;
}