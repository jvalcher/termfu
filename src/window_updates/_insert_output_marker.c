
/*
    Insert start/end marker with plugin output file path / code into debugger process output
*/

#include <unistd.h>

#include "../data.h"
#include "../utilities.h"

static void gdb_insert_output_start_marker (char *path, state_t *state);
static void gdb_insert_output_end_marker (char *code, state_t *state);

ssize_t mark_bytes;



/********************************
  Debugger output stream markers
 ********************************/

/*
    >START:<output_file_path>:
*/
void insert_output_start_marker (char    *code, 
                                 state_t *state)
{
    // get plugin
    plugin_t *curr_plugin = state->plugins;
    do {
        if (strcmp (code, curr_plugin->code) == 0) break;
        curr_plugin = curr_plugin->next;
    } while (curr_plugin != NULL);

    // set start marker
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_insert_output_start_marker (curr_plugin->window->out_file_path, state);
            break;
    }
}


/*
    >END:<plugin_code>:
*/
void insert_output_end_marker (char    *code, 
                               state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_insert_output_end_marker (code, state);
            break;
    }
}


/*
    TODO: Window commands marker (?)
    - Single marker that includes code
    - select, deselect, ...
*/



/**************************
  Debugger implementations
 **************************/


static void gdb_insert_output_start_marker (char *path,
                                            state_t *state)
{
    char *start = "echo >START:";
    mark_bytes  = write (INPUT_PIPE, start, strlen (start));
    mark_bytes += write (INPUT_PIPE, path, strlen (path));
    mark_bytes += write (INPUT_PIPE, ":\n", 2);
    CHECK_BYTES(mark_bytes);
}



/*
    Insert end mark, plugin code
*/
static void gdb_insert_output_end_marker (char *code,
                                          state_t *state)
{
    char *end = "echo >END:";
    mark_bytes  = write (INPUT_PIPE, end, strlen (end));
    mark_bytes += write (INPUT_PIPE, code, strlen (code));
    mark_bytes += write (INPUT_PIPE, ":\n", 2);
    CHECK_BYTES(mark_bytes);
}



