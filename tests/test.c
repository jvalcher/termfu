#include <stdio.h>
#include <stdlib.h>


int
main (void)
{
    typedef struct {
        int index;
    } test_struct;

    test_struct *ts = (test_struct*) malloc (sizeof (test_struct));

    ts->index = 4;

    printf ("%d\n", ts->index);

    return 0;
}
