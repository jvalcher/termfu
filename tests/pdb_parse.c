#include "../src/data.h"
#include "../src/parse_debugger_output.h"


int
main (void)
{
    state_t *state = (state_t*) malloc (sizeof (state_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*));
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    reader_t reader;
    FILE *fp;
    int ch, i;

    state->debugger->cli_buffer[0] = '\0';
    state->debugger->program_buffer[0] = '\0';
    state->debugger->data_buffer[0] = '\0';
    state->debugger->async_buffer[0] = '\0';

    reader.cli_buffer_ptr = state->debugger->cli_buffer;
    reader.program_buffer_ptr = state->debugger->program_buffer;
    reader.data_buffer_ptr = state->debugger->data_buffer;
    reader.async_buffer_ptr = state->debugger->async_buffer;

    // open pdb.out
    if ((fp = fopen ("pdb.out", "r")) == NULL) {
        perror ("Debugger output file");
        exit (1);
    }

    // transfer to reader
    i = 0;
    while ((ch = fgetc (fp)) != EOF) {
        reader.output_buffer [i++] = ch;
    }
    reader.output_buffer [i] = '\0';

    // parse output
    parse_debugger_output_pdb (&reader);

    printf ("CLI:\n\n'''\n%s\n'''\n\n", state->debugger->cli_buffer);
    printf ("PROGRAM: \n\n'''\n%s\n'''\n\n", state->debugger->program_buffer);


    return 0;
}

