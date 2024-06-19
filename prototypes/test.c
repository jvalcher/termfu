#include <stdio.h>


void change (int *num)
{
    *num = 8;
}

void change_helper (int *num)
{
    change (num);
}

int main (void)
{
    int num = 4;

    change_helper (&num);

    printf ("%d\n", num);
     
    return 0;
}
