#include <stdio.h>

#include "../src/utilities.h"


/*
    perr()
*/
int main (void)
{
    pfem  ("Test error");
    pfemo ("var1 = 5");
    pfemo ("var2 = 10");

    puts("");
    
    char ch_err_mes[] = "Unknown window layout symbol (_)";
    ch_err_mes [strlen(ch_err_mes) - 2] = 'c';
    pfem (ch_err_mes);

    return 0;
}
