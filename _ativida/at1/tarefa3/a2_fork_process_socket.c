#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define SOCKET_PATH "/tmp/fork_example_socket"
#define BUFFER_SIZE 128

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

// Socket descriptors
int server_sock = -1;
int client_sock = -1;

// Initialize Unix domain socket
bool init_lock() {
    struct sockaddr_un server_addr;
    
    // Create socket
    if ((server_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return false;
    }
    
    // Initialize address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
    
    // Remove socket file if it already exists
    unlink(SOCKET_PATH);
    
    // Bind socket to address
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_sock);
        return false;
    }
    
    // Listen for connections (queue up to 5 connection requests)
    if (listen(server_sock, 5) == -1) {
        perror("listen");
        close(server_sock);
        return false;
    }
    
    return true;
}

// Parent function - acts as server
bool parent_wait_for_message(char *message, size_t msg_size) {
    struct sockaddr_un client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    printf("[P] Waiting for connection from child\n");
    
    // Accept connection from client (child)
    if ((client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len)) == -1) {
        perror("accept");
        return false;
    }
    
    printf("[P] Connection accepted from child\n");
    
    // Receive message from client
    ssize_t bytes_received = recv(client_sock, message, msg_size - 1, 0);
    if (bytes_received == -1) {
        perror("recv");
        close(client_sock);
        return false;
    }
    
    // Null-terminate the message
    message[bytes_received] = '\0';
    
    // Close the client socket
    close(client_sock);
    
    return true;
}

// Child function - acts as client
bool child_send_message(const char *message) {
    struct sockaddr_un server_addr;
    int sock;
    
    // Create socket
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return false;
    }
    
    // Initialize address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
    
    // Connect to server (parent)
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sock);
        return false;
    }
    
    // Send message to server
    if (send(sock, message, strlen(message), 0) == -1) {
        perror("send");
        close(sock);
        return false;
    }
    
    // Close the socket
    close(sock);
    
    return true;
}

// Clean up socket
void cleanup_socket() {
    if (client_sock != -1) {
        close(client_sock);
        client_sock = -1;
    }
    
    if (server_sock != -1) {
        close(server_sock);
        server_sock = -1;
    }
    
    unlink(SOCKET_PATH);
}

void experiment(struct Experiment *e) {
    // Initialize the socket
    if (!init_lock()) {
        fprintf(stderr, "Failed to initialize socket\n");
        return;
    }

    pid_t child_pid = fork();
    char fc[2];
    char message[BUFFER_SIZE];

    switch (check_fork(child_pid)) {
        case FORK_FAIL:
            perror("fork");
            cleanup_socket();
            break;

        case FORK_CHILD:
            strcpy(fc, "C");
            print_process(fc);
            sleep_process(fc, e->sleep_child);
            
            // Child sends message to parent
            printf("[C] Sending message to parent via socket\n");
            child_send_message("CHILD_DONE");
            break;

        case FORK_PARENT:
            strcpy(fc, "P");
            print_process(fc);
            sleep_process(fc, e->sleep_parent);
            
            // Parent waits for message from child
            parent_wait_for_message(message, BUFFER_SIZE);
            printf("[P] Received message from child: %s\n", message);
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
        exit(0);
    } else if (check_fork(child_pid) == FORK_PARENT) {
        cleanup_socket();
    }
}

int main() {
    struct Experiment experiments[] = {
        {2, "Socket Synchronization", 1, 3, 1, true},
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
    
    return 0;
}