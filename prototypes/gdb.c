#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>

#include "data.h"

char *flush_cmd  = "call fflush (0)\n";
char *set_tty = "-inferior-tty-set /dev/pts/6\n";


/*
    Codes
    -----

termide_back,       "Bak"
termide_builds,     "Bld"
termide_layouts,    "Lay"

gdb_assembly,       "Asm"
gdb_breakpoints,    "Brk"
gdb_continue,       "Con"
gdb_finish,         "Fin"
gdb_kill,           "Kil"
gdb_local_vars,     "LcV"
gdb_next,           "Nxt"
gdb_output,         "Out"
gdb_prompt,         "Prm"
gdb_print,          "Prn"
gdb_registers,      "Reg"
gdb_run,            "Run"
gdb_src_file,       "Src"
gdb_step,           "Stp"
gdb_watches         "Wat"

    TODO:
    ----

    - Add mutex that sets when writing to pipe and doesn't unlock until (gdb) prompt seen in output process

    -interpreter-exec  for typed commands
*/


int create_output_start_marker (debug_state_t *dstate, char *path)
{
    ssize_t  bytes    = 0;
    char *start = "echo >START:";

    // send output done string for debugger reader thread
    bytes += write (dstate->input_pipe, start, strlen (start));
    bytes += write (dstate->input_pipe, path, strlen (path));
    bytes += write (dstate->input_pipe, ":\n", 1);

    return (int)bytes;
}



int create_output_end_marker (debug_state_t *dstate)
{
    ssize_t  bytes    = 0;
    char *echo = "echo >END:";

    // send output done string for debugger reader thread
    bytes += write (dstate->input_pipe, dstate->out_done_str, strlen (dstate->out_done_str));
    bytes += write (dstate->input_pipe, "\n", 1);

    return (int)bytes;
}


int gdb_run (debug_state_t *state)
{
    ssize_t  bytes    = 0;
    //char    *set_tty  = "-inferior-tty-set ";
    char    *gdb_run  = "-exec-run\n";

    /*
    // set program stdout to current tty
    char *tty_device = ttyname(STDIN_FILENO);
    bytes += write (state->input_pipe, set_tty,  strlen (set_tty));
    bytes += write (state->input_pipe, tty_device,  strlen (tty_device));
    bytes += write (state->input_pipe, "\n",  strlen ("\n"));
    */

    // run program
    bytes += write (state->input_pipe, gdb_run, strlen (gdb_run));

    return (int)bytes;
}


int gdb_set_breakpoint (debug_state_t *state)
{
    ssize_t bytes       = 0;
    char *set_break_cmd = "-break-insert ",
         *break_loc     = state->break_point;

    // set breakpoint
    bytes += write (state->input_pipe, set_break_cmd, strlen (set_break_cmd));
    bytes += write (state->input_pipe, break_loc, strlen (break_loc));
    bytes += write (state->input_pipe, "\n", 1);

    return (int)bytes;
}


int gdb_next (debug_state_t *state)
{
    ssize_t bytes;
    char *next_cmd = "-exec-next\n";

    bytes = write (state->input_pipe, next_cmd, strlen (next_cmd));

    return (int)bytes;
}


int gdb_continue (debug_state_t *state)
{
    ssize_t bytes;
    char *cont_cmd = "-exec-continue\n";

    bytes = write (state->input_pipe, cont_cmd, strlen (cont_cmd));

    return (int)bytes;
}


int gdb_exit (debug_state_t *state)
{
    ssize_t bytes;
    char *exit_cmd = "-gdb-exit\n";

    bytes =  write (state->input_pipe, exit_cmd, strlen (exit_cmd));

    return (int)bytes;
}


/*
   Non-plugin commands
*/


int gdb_get_local_vars (debug_state_t *state)
{
    ssize_t bytes;
    char *exit_cmd = "info locals\n";

    bytes = write (state->input_pipe, exit_cmd, strlen (exit_cmd));

    return (int)bytes;
}


/*
    Update window data
*/



FILE *clear_and_open_file_for_append (char *path)
{
    FILE *fp = fopen (path, "w");
    fclose (fp);
    return fopen (path, "a");
}


/*
    Parse GDB output
*/
void gdb_parse_output (char *in_buffer, char *debug_out_path, char *program_out_path)
{
    bool  is_gdb_output,
          is_prog_output,
          is_newline,
          is_new_string;
    FILE *debug_out_ptr,
         *program_out_ptr;
    char *buff_ptr;
    int   i;

    buff_ptr = in_buffer;
    
    debug_out_ptr   = fopen (debug_out_path, "a");
    program_out_ptr = fopen (program_out_path, "a");

    is_gdb_output = false,
    is_prog_output = false,
    is_newline = true,
    is_new_string = false;

    while (*buff_ptr != '\0') {

        // set type of output for new line
        if (is_newline && *buff_ptr != '\n') {
            is_newline = false;
            if (*buff_ptr == '~') {
                is_gdb_output = true;
                ++buff_ptr;
            } 
            else if (isalpha (*buff_ptr)) {
                is_prog_output = true;
            }
        }

        // end of line
        else if (*buff_ptr == '\n') {
            is_newline = true;

            // gdb
            if (is_gdb_output) {
                is_gdb_output = false;
                fputc (*buff_ptr++, debug_out_ptr);
            }

            // program
            else if (is_prog_output) {
                is_prog_output = false;
                fputc (*buff_ptr++, program_out_ptr);
            }

            else {
                ++buff_ptr;
            }
        }

        // gdb output
        else if (is_gdb_output) {

            //  if  \\\t,\n,\_  or  \\\n  -> skip
            if (*buff_ptr == '\\' && (isalpha(*(buff_ptr + 1)) || *(buff_ptr + 1) == '\n') ) {
                buff_ptr += 2;
            }

            //  if  \\\"  ->  \"
            else if (*buff_ptr == '\\' && *(buff_ptr + 1) == '\"' ) {
                buff_ptr += 1;
                fputc (*buff_ptr++, debug_out_ptr);
            }

            //  if  \"  ->  skip
            else if (*buff_ptr == '\"') {
                buff_ptr += 1;
            }

            //  if  >START  -> set debug out path
            else if ( *buff_ptr    == '>' && 
                     *(buff_ptr + 1) == 'S' && 
                     *(buff_ptr + 2) == 'T' && 
                     *(buff_ptr + 3) == 'A' && 
                     *(buff_ptr + 3) == 'R' && 
                     *(buff_ptr + 4) == 'T') {
                buff_ptr += 7;
                i = 0;
                while (*buff_ptr != ':') {
                    debug_out_path [i++] = *buff_ptr++;
                }
                debug_out_path [i] = '\0';
                debug_out_ptr = clear_and_open_file_for_append (debug_out_path);
            } 

            // if  >END   -> send signal
            else if ( *buff_ptr      == '>' && 
                     *(buff_ptr + 1) == 'E' && 
                     *(buff_ptr + 2) == 'N' && 
                     *(buff_ptr + 3) == 'D') {
                buff_ptr += 4;
                puts ("END");
                break;
            }

            else {
                fputc (*buff_ptr++, debug_out_ptr);
            }
        }

        // program output
        else if (is_prog_output) {
            fputc (*buff_ptr++, program_out_ptr);
        }

        else {
            ++buff_ptr;
        }
    }

    fclose (debug_out_ptr);
    fclose (program_out_ptr);
}

