
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../src/utilities.h"


int main (void)
{
    int   n;
    char *src_str = "abcdefghi",
          dst_str[6];

    n = strncpy (dst_str, src_str, strlen (src_str) - 1);

    printf ("%d\n", n);

    return 0;
}
