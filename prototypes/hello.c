#include <stdio.h>
#include <stdlib.h>

struct data {
    char *greet;
    int var1;
};

int main (void)
{
    char *hello = "Hello, Jeffrey!";
    int age = 37;
    struct data data1;
    data1.greet = hello;
    data1.var1 = age;

    printf ("Program started\n");
    printf ("%s\n", data1.greet);
    printf ("Program ended\n");

    return EXIT_SUCCESS;
}
