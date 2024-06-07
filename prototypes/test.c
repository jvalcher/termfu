#include <stdio.h>
#include <unistd.h>

int main() {
    // Get the current TTY device
    char *tty_device = ttyname(STDIN_FILENO);

    if (tty_device != NULL) {
        printf("Current TTY device: %s\n", tty_device);
    } else {
        perror("ttyname");
        return 1;
    }

    return 0;
}
