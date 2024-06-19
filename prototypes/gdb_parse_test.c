#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>

#include "gdb.h"

void check_file_equality (char *title, FILE *check_fp, FILE *correct_fp);

int main() 
{
    char buffer [8192];
    char ch;
    int  i;

    char *dbg_out_path = "./out_debug.out";
    char *prg_out_path = "./out_prog.out";

    // erase out files
    FILE *out_dbg_fp = fopen ("./out_debug.out", "w");
    FILE *out_prg_fp = fopen ("./out_prog.out", "w");
    fclose (out_dbg_fp);
    fclose (out_prg_fp);

    // buffer input file
    FILE *in_fp = fopen ("./mock_data.in", "r");
    i = 0;
    while ((ch = fgetc (in_fp)) != EOF) {
        buffer [i++] = ch;
    }
    buffer [i] = '\0';

    // parse into debugger, program output files
    gdb_parse_output (buffer, dbg_out_path, prg_out_path);

    // check output files results
    out_dbg_fp = fopen ("./out_debug.out", "r");
    out_prg_fp = fopen ("./out_prog.out", "r");
    FILE *mock_dbg_fp = fopen ("./mock_debug.out", "r");
    FILE *mock_prg_fp = fopen ("./mock_prog.out", "r");
    check_file_equality ("Program", out_prg_fp, mock_prg_fp);
    check_file_equality ("Debug", out_dbg_fp, mock_dbg_fp);

    fclose (in_fp);
    fclose (out_dbg_fp);
    fclose (out_prg_fp);
    fclose (mock_dbg_fp);
    fclose (mock_prg_fp);

    return 0;
}



void check_file_equality (char *title, FILE *check_fp, FILE *correct_fp)
{
    int i, j, nl, chn;
    char ch1, ch2;
    // check program file results
    puts ("");
    puts ("-----------");
    i = 0;
    j = 0;
    nl = 1;
    chn = 1;
    while ((ch1= fgetc (check_fp)) != EOF && (ch2 = fgetc (correct_fp)) != EOF) {
        if (ch1 != ch2) {
            printf ("Line %d, character %d:  %c !=  %c\n", nl, chn, ch1, ch2);
            printf ("%s", title);
            printf (" file incorrect\n");
            break;
        }
        ++chn;
        if (ch1 == '\n') {
            ++nl;
            chn = 1;
        }
    }
    if (ch1 == EOF) {
        printf ("%s", title);
        printf (" file correct\n");
    }
    puts ("-----------");
    i = 0;
    rewind (check_fp);
    while ((ch1 = fgetc (check_fp)) != EOF) {
        putchar (ch1);
    }
    puts ("-----------");
    puts ("");
}
