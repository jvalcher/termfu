#include "../src/utilities.h"



int
main (void)
{
    char *str1 = "Hello, ",
         *str2 = "world",
         *str3 = "!",
         *str;

    str = concatenate_strings (str1, str2, str3);
    printf ("\"%s\"\n", str);
    free (str);

    return 0;
}
