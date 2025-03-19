#include "../src/error.h"

extern int errno;

int
main (void)
{
    errno = 1;

    pfem ("Error!");

    return 0;
}
