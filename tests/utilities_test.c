/* 
    utilities.c,h
*/

#include <stdio.h>

#include "../src/utilities.h"


int main (void)
{
    int ch = 't';
    pfem  ("Unknown character %c\n", ch);
    pfemo ("var1 = %d\n", 5);

    return 0;
}
