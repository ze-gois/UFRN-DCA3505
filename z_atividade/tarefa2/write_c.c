#include <unistd.h>

int main() {
    const char *message = "Grato por tudo.\n";
    ssize_t result = write(STDOUT_FILENO, message, 16);

    return 0;
}
