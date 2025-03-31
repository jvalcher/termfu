#include <stdio.h>

int main() {
    int *ptr = NULL;  // Create a NULL pointer

    // Attempt to dereference the NULL pointer
    *ptr = 42;

    printf("Value at ptr: %d\n", *ptr);
    return 0;
}

