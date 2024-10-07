#include "../src/utilities.h"



int
main (void)
{
    char *str1 = "Hello, ",
         *str2 = "world",
         *str3 = "!\n",
         *str;

    str = concatenate_strings (3, str1, str2, str3);

    printf ("\"%s\"", str);

    return 0;
}
