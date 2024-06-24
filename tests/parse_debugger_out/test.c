
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>

#include "../utilities.h"
#include "../../src/data.h"
#include "../../src/parse_debugger_output.h"

typedef struct tdata {
    int   debugger;
    char *plugin_code;
    char *data_in_path;
    char *debug_out_path;
    char *program_out_path;
    char *correct_debug_out_path;
    char *correct_program_out_path;
} tdata_t;

void run_parse_test (tdata_t*);
void check_file_equality (char*, char*, FILE*, FILE*);


// data
int   debugger []             = {    DEBUGGER_GDB,           DEBUGGER_GDB,             DEBUGGER_GDB,          };
char  plugin_code [][4]       = {      "Run",                  "Brk",                    "Wat",               };
char *debug_out_path []       = { "./d/_debug.out",       "./d/_debug.out",         "./d/_debug.out",         };
char *progr_out_path []       = { "./d/_program.out",     "./d/_program.out",       "./d/_program.out",       };
char *data_in_path []         = { "./d/run.in",           "./d/break.in",           "./d/watch.in",           };
char *correct_dbg_out_path [] = { "./d/run_corr_dbg.out", "./d/break_corr_dbg.out", "./d/watch_corr_dbg.out", };
char *correct_prg_out_path [] = { "./d/run_corr_prg.out", "./d/break_corr_prg.out", "./d/watch_corr_prg.out", };



int main() 
{
    int i;
    int  num_tests = sizeof (debugger) / sizeof (debugger [0]);
    tdata_t *tdata = (tdata_t*) malloc (sizeof (tdata_t));

    puts ("");
    for (i = 0; i < num_tests; i++) {
        
        tdata->debugger                 = debugger [i]; 
        tdata->plugin_code              = plugin_code [i];
        tdata->data_in_path             = data_in_path [i];
        tdata->debug_out_path           = debug_out_path [i];
        tdata->program_out_path         = progr_out_path [i];
        tdata->correct_debug_out_path   = correct_dbg_out_path [i];
        tdata->correct_program_out_path = correct_prg_out_path [i];

        run_parse_test (tdata);
    }

    return 0;
}



void run_parse_test (tdata_t *tdata)
{
    int  i,
         reader_state;
    char ch,
         in_buffer [8192],
         debug_out_path [256],
         plugin_code [4];
    FILE *in_fp,
         *out_dbg_fp,
         *out_prg_fp,
         *correct_dbg_fp,
         *correct_prg_fp;

    reader_state = -1;

    // erase out files
    out_dbg_fp = fopen (tdata->debug_out_path, "w");
    out_prg_fp = fopen (tdata->program_out_path, "w");
    fclose (out_dbg_fp);
    fclose (out_prg_fp);

    // buffer input file
    in_fp = fopen (tdata->data_in_path, "r");
    i = 0;
    while ((ch = fgetc (in_fp)) != EOF) {
        in_buffer [i++] = ch;
    }
    in_buffer [i] = '\0';
    fclose (in_fp);

    // parse
    parse_debugger_output (tdata->debugger, 
                           &reader_state,
                           in_buffer,
                           plugin_code,
                           debug_out_path,
                           tdata->program_out_path);

    // check results
    START_TEST(tdata->plugin_code);

        // output path
    if (strcmp (debug_out_path, tdata->debug_out_path) == 0) {
        TEST_PASSED;
        printf ("Debug output path: \"%s\" == \"%s\"\n", debug_out_path, tdata->debug_out_path);
    } else {
        TEST_FAILED;
        printf ("Debug output path: \"%s\" != \"%s\"\n", debug_out_path, tdata->debug_out_path);
    }

        // reader state
    if (reader_state > 0) {
        TEST_PASSED;
        printf ("Reader state: %d > 0\n", reader_state);
    } else {
        TEST_FAILED;
        printf ("Reader state: %d < 0\n", reader_state);
    }

        // plugin code
    if (strcmp (plugin_code, tdata->plugin_code) == 0) {
        TEST_PASSED;
        printf ("Plugin code: \"%s\" == \"%s\"\n", plugin_code, tdata->plugin_code);
    } else {
        TEST_FAILED;
        printf ("Plugin code: \"%s\" != \"%s\"\n", plugin_code, tdata->plugin_code);
    }

        // debugger, program output files
    out_dbg_fp     = fopen (tdata->debug_out_path, "r");
    out_prg_fp     = fopen (tdata->program_out_path, "r");
    correct_dbg_fp = fopen (tdata->correct_debug_out_path, "r");
    correct_prg_fp = fopen (tdata->correct_program_out_path, "r");
    check_file_equality (tdata->data_in_path, tdata->debug_out_path,   out_dbg_fp, correct_dbg_fp);
    check_file_equality (tdata->data_in_path, tdata->program_out_path, out_prg_fp, correct_prg_fp);
    fclose (out_dbg_fp);
    fclose (out_prg_fp);
    fclose (correct_dbg_fp);
    fclose (correct_prg_fp);

    END_TEST(tdata->plugin_code);
    puts ("");
}



void check_file_equality (char *in_path, char *check_path, FILE *check_fp, FILE *correct_fp)
{
    int nl, chn;
    char ch1, ch2;

    // check program file results
    nl = 1;
    chn = 1;
    while ((ch1= fgetc (check_fp)) != EOF && (ch2 = fgetc (correct_fp)) != EOF) {

        // output file incorrect
        if (ch1 != ch2) {
            TEST_FAILED;
            printf ("Input path:  \"%s\"\n", in_path);
            printf ("Output path: \"%s\"\n\n", check_path);
            printf ("    Line %d, character %d:  \"%c\" !=  \"%c\"\n", nl, chn, ch1, ch2);
            break;
        }

        ++chn;
        if (ch1 == '\n') {
            ++nl;
            chn = 1;
        }
    }

    // output file correct
    if (ch1 == EOF) {
        TEST_PASSED;
        printf ("Input path:  \"%s\"\n", in_path);
        printf ("Output path: \"%s\"\n", check_path);
    } 

    // output incorrect, print file contents
    else {
        puts ("-----------");
        rewind (check_fp);
        while ((ch1 = fgetc (check_fp)) != EOF) {
            putchar (ch1);
        }
        puts ("-----------");
    }
}
