#include <unistd.h>

int main() {
    const char *message = "Hello from C program using write syscall\n";
    ssize_t result = write(STDOUT_FILENO, message, 38);
    
    return 0;
}