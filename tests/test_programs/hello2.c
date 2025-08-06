
#include <stdio.h>

#include "hello2.h"


void print_message (int n)
{
    const char* msg = "Have a wonderful day!";

    printf ("There are %d days left before the event. %s\n", n, msg);
}

