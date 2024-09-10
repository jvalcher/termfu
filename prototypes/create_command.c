#include "stdio.h"
#include <stdlib.h>
#include <string.h>



static char**
create_command (char *cmd_str)
{
    char **cmd_arr,
          *cp,
           buff [48];
    int n, i;

    
    // count words
    cp = cmd_str;
    n = 1;  
    while (*cp != '\0') {
        if (*cp == ' ') {
            ++n;
        }
        ++cp;
    }
    ++n;    // execvp NULL

    // create array
    cmd_arr = (char**) malloc (n * sizeof (char*));
        //
    cp = cmd_str;
    n = 0;  
    i = 0;
    while (*cp != '\0') {
        if (*cp == ' ') {
            ++cp;
            i = 0;
        }
        else {
            do {
                buff [i++] = *cp++;
            } while (*cp != ' ' && *cp != '\0');
            buff [i] = '\0';
                //
            cmd_arr [n] = (char*) malloc (strlen (buff) + 1);
            strcpy (cmd_arr [n++], buff); 
        }
    }
    cmd_arr [n] = NULL;

    return cmd_arr;
}



int
main (void)
{
    char *cmd_str = "gdb --quiet --interpreter=mi ../misc/hello";
    char **cmd_arr;
    int i;

    cmd_arr = create_command (cmd_str);

    i = 0;
    do {
        printf ("%s ", cmd_arr [i++]);
    } while (cmd_arr [i] != NULL);
    puts (" ");

    return 0;
}


