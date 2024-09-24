#include <ncurses.h>
#include <string.h>
#include <errno.h>


int
main (void)
{
    int i;
    char *cmd[] = {"gdb", "--interpreter=mi", "a.out", NULL};

    i = 0;
    do {
        printf ("%s ", cmd [i++]);
    } while (cmd [i] != NULL);

    return 0;
}
