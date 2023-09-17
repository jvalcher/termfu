
/*
   GDB/MI interface
*/


#include <stdio.h>


struct gdb_state {
    int line;
    char *source_file;
} gdb_state;


/*
    Get source code location
    ------------------------
    (gdb) info line
    Line 12 of "test.c" starts at address 0x55555555517e <main+53> (...)
         ^^     ^^^^^^



void get_location (void) 
{
    return 0;
}

*/

