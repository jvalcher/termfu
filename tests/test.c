
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>


char*
concat_strings (int num_strs, ...)
{
    char  buffer [256] = {0},
         *sub_str,
         *str;
    va_list strs;

    va_start (strs, num_strs);

    for (int i = 0; i < num_strs; i++) {
        sub_str = va_arg (strs, char*);
        strncat (buffer, sub_str, sizeof(buffer) - strlen(buffer) - 1);
    }
    va_end (strs);

    str = (char*) malloc (strlen (buffer) + 1);
    if (str == NULL) {
        fprintf (stderr, "String malloc failed");
        exit (EXIT_FAILURE);
    }
    strcpy (str, buffer);

    return str;
}



int main (void)
{
    char *str1 = "I love sarcasm. ";
    char *str2 = "It\'s like punching people in the face, ";
    char *str3 = "but with words.";

    char *str = concat_strings (3, str1, str2, str3);

    printf ("%s\n", str);

    free (str);

    return 0;
}
