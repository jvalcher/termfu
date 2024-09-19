#include <stdlib.h>

#include "../src/data.h"
#include "../src/parse_debugger_output.h"


int
main (void)
{
    state_t *state = (state_t*) malloc (sizeof (state_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*));
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    FILE *fp;
    int ch, i;

    state->debugger->index = DEBUGGER_PDB;
    state->debugger->cli_pos = 0;
    state->debugger->cli_len = CLI_BUF_LEN;

    // open pdb.out
    if ((fp = fopen ("pdb.out", "r")) == NULL) {
        perror ("Debugger output file");
        exit (1);
    }

    // transfer to reader
    i = 0;
    while ((ch = fgetc (fp)) != EOF) {
        state->debugger->reader_buffer [i++] = ch;
    }
    state->debugger->reader_buffer [i] = '\0';

    // parse output
    parse_debugger_output_pdb (state->debugger);

    printf ("\n\nCLI:\n\n'''\n%s\n'''\n\n", state->debugger->cli_buffer);


    return 0;
}

