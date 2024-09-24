#include <stdio.h>
#include <errno.h>

#include "../src/utilities.h"

int
main (void)
{
    FILE *fp;
    int x, y;

    // fopen
    char *path = "fake.txt";
    if ((fp = fopen(path, "r")) == NULL) {
        pfem ("File \"%s\" not found: %s", path, strerror (errno));
    }

    // malloc
    pfem ("malloc: %s", strerror (ENOMEM));
    pem  ("Failed to allocate memory");

    // divide by zero
    x = 5;
    y = 0;
    if (y == 0) {
        pfeme ("Divide by zero (y == 0): %s\n", strerror (EDOM));
    } else {
        printf ("x / y = %d\n", x/y);
    }

    printf ("Program finished execution\n");

    return 0;
}
