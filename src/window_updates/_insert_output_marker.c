
/*
    Insert start/end marker with plugin output file path / code into debugger process output
*/

#include <unistd.h>

#include "../data.h"
#include "../utilities.h"

static void  gdb_insert_output_start_marker         (char*, char*, state_t*);
static void  gdb_insert_output_end_marker           (state_t *state);
static void  gdb_insert_output_win_select_marker    (char*, state_t*);
static void  gdb_insert_output_win_deselect_marker  (char*, state_t*);

ssize_t mark_bytes;



/********************************
  Debugger output stream markers
 ********************************/

/*
    >START:<plugin code>:<output_file_path>:
*/
void 
insert_output_start_marker (char    *code, 
                            state_t *state)
{
    char *path = get_code_path (code, state->plugins);
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB): gdb_insert_output_start_marker (path, code, state); break;
    }
}


/*
    >END
*/
void
insert_output_end_marker (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB): gdb_insert_output_end_marker (state); break;
    }
}


/*
    >WSEL:<plugin code>:
*/
void 
insert_output_win_select_marker (char    *code, 
                                 state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB): gdb_insert_output_win_select_marker (code, state); break;
    }
}


/*
    >WDSL:<plugin code>:
*/
void 
insert_output_win_deselect_marker (char    *code, 
                                   state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB): gdb_insert_output_win_deselect_marker (code, state); break;
    }
}



/**************************
  Debugger implementations
 **************************/


static void 
gdb_insert_output_start_marker (char    *path,
                                char    *code,
                                state_t *state)
{
    char *start = "echo >START:";
    mark_bytes  = write (INPUT_PIPE, start, strlen (start));
    mark_bytes += write (INPUT_PIPE, code, strlen (code));
    mark_bytes += write (INPUT_PIPE, ":", 1);
    mark_bytes += write (INPUT_PIPE, path, strlen (path));
    mark_bytes += write (INPUT_PIPE, ":\n", 2);
    CHECK_BYTES(mark_bytes);
}



/*
    Insert end mark, plugin code
*/
static void
gdb_insert_output_end_marker (state_t *state)
{
    char *end = "echo >END\n";
    mark_bytes  = write (INPUT_PIPE, end, strlen (end));
    CHECK_BYTES(mark_bytes);
}



static void 
gdb_insert_output_win_select_marker (char    *code,
                                     state_t *state)
{
    char *start = "echo >WSEL:";
    mark_bytes  = write (INPUT_PIPE, start, strlen (start));
    mark_bytes += write (INPUT_PIPE, code, strlen (code));
    mark_bytes += write (INPUT_PIPE, ":\n", 2);
    CHECK_BYTES(mark_bytes);
}



static void 
gdb_insert_output_win_deselect_marker (char    *code,
                                       state_t *state)
{
    char *start = "echo >WDSL:";
    mark_bytes  = write (INPUT_PIPE, start, strlen (start));
    mark_bytes += write (INPUT_PIPE, code, strlen (code));
    mark_bytes += write (INPUT_PIPE, ":\n", 2);
    CHECK_BYTES(mark_bytes);
}



