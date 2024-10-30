#include "../src/utilities.h"

#define MOCK_WATCH  "mock.gdb_win_watch"



int
main (void)
{
    char *buff = create_buff_from_file (MOCK_WATCH);

    printf ("%s\n", buff);

    free (buff);

    return 0;
}
