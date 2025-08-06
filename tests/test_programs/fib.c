/*

 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

void are_equal(int x, int y);
void strs_are_equal(char *x, char *y);


// solution

/*  first attempt  2ms
int fib(int n)
{
    int table[32];

    table[0] = 0;
    table[1] = 1;

    for (int i = 2; i <= n; i++)
    {
        table[i] = table[i-1] + table[i-2];
    }
    return table[n];
}
*/

/*
    second attempt 0ms
*/
static int table[32] = {0, 1};
static int i = 2;

int fib(int n)
{
    for (; i <= n; i++)
    {
        table[i] = table[i-1] + table[i-2];
    }
    return table[n];
}



// tests
int main (void) 
{
    printf ("%d\n", fib(2));
    printf ("%d\n", fib(3));
    printf ("%d\n", fib(4));

    return 0;
}



void ints_are_equal(int x, int y)
{
    if (x == y)
        printf("\033[1;32m\u2713\033[1;0m %d == %d\n", x, y);
    else
        printf("\033[1;31mx\033[1;0m %d != %d\n", x, y);
}

void strs_are_equal(char *x, char *y)
{
    if (strcmp(x, y) == 0)
        printf("\033[1;32m\u2713\033[1;0m %s == %s\n",x, y);
    else
        printf("\033[1;31mx\033[1;0m %s != %s\n", x, y);
}


