#include <stdio.h>

#include "hello2.h"

struct data_s {
    char *str;
    int num;
    char ch;
};

int main (void) 
{
    int   n1      = 13,
          n_arr[] = {1, 2, 3, 4, 5},
          n_len   = sizeof (n_arr) / sizeof (n_arr[0]);
            
    char *str1 = "Hello, ",
         *str2 = "world!\n",
         *str3 = "This is a long line of \"characters\" that might not fit in the window";

    struct data_s ds = {str3, n1, 'a'};

    printf ("%s", str1);
    printf ("%s", str2);
    printf ("%s\n", str3);
    printf ("%d %c %s\n", ds.num, ds.ch, ds.str);

    for (int i = 0; i < n_len; i++) {
        n1 += n_arr [i];
    }

    print_message (n1);

    return 0;
}

