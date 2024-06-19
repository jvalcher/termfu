
/*
    GDB commands
*/

#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>

#include "gdb.h"
#include "_utilities.h"
#include "_interface.h"
#include "../data.h"
#include "../utilities.h"


ssize_t bytes;
int ch;


/*
    --------------
    Header plugins
    --------------
*/
void gdb_run (state_t *state)
{
    char *tty_dev,
         *tty_cmd   = "-inferior-tty-set ",
         *run_cmd   = "-exec-run\n",
         *prog_path = state->debug_state->prog_path;

    // set program stdout to current tty
    if ((tty_dev = ttyname(STDIN_FILENO)) == NULL) {
        pfeme ("Failed to set tty");
    }
    bytes  = write (INPUT_PIPE, tty_cmd,  strlen (tty_cmd));
    bytes += write (INPUT_PIPE, tty_dev,  strlen (tty_dev));
    bytes += write (INPUT_PIPE, "\n", 1);

    // run prog_path
    bytes += write (INPUT_PIPE, gdb_run, strlen (run_cmd));
    bytes += write (INPUT_PIPE, prog_path, strlen (prog_path));
    bytes += write (INPUT_PIPE, "\n", strlen ("\n"));
    CHECK_BYTES(bytes);
}


void gdb_pwin_set_breakpoint (state_t *state)
{
    char *break_cmd = "-break-insert ";

    // TODO: 
        // popup window to enter breakpoint line number, string
    char *breakpoint = "main";

    bytes  = write (INPUT_PIPE, break_cmd, strlen (break_cmd));
    bytes += write (INPUT_PIPE, breakpoint, strlen (breakpoint));
    bytes += write (INPUT_PIPE, "\n", 1);
    CHECK_BYTES(bytes);
}


void gdb_next (state_t *state)
{
    char *next_cmd = "-exec-next\n";
    bytes = write (INPUT_PIPE, next_cmd, strlen (next_cmd));
    CHECK_BYTES(bytes);
}


void gdb_step (state_t *state) 
{
    char *step_cmd = "-exec-step\n";
    bytes = write (INPUT_PIPE, step_cmd, strlen (step_cmd));
    CHECK_BYTES(bytes);
}


void gdb_continue (state_t *state)
{
    char *cont_cmd = "-exec-continue\n";
    bytes = write (INPUT_PIPE, cont_cmd, strlen (cont_cmd));
    CHECK_BYTES(bytes);
}


void gdb_finish (state_t *state) 
{
    char *fin_cmd = "-exec-finish\n";
    bytes = write (INPUT_PIPE, fin_cmd, strlen (fin_cmd));
    CHECK_BYTES(bytes);
}



void gdb_kill (state_t *state)
{
    char *kill_cmd = "-exec-abort\n";
    bytes = write (INPUT_PIPE, kill_cmd, strlen (kill_cmd));
    CHECK_BYTES(bytes);
}



void gdb_exit (state_t *state)
{
    char *dbg_quit = "-gdb-exit\n";
    char *rdr_quit = "echo >EXIT\n";

    // exit debugger process
    bytes = write (INPUT_PIPE, dbg_quit, strlen (dbg_quit));

    // exit debugger reader process
    bytes += write (INPUT_PIPE, rdr_quit, strlen (rdr_quit));

    CHECK_BYTES(bytes);
}



/****************
  Window plugins
 ****************/


void gdb_win_assembly (state_t *state)
{
    while ((ch = getchar()) != state->win_keys->back) {
        run_other_win_key (ch, state);
    }
}



void gdb_win_prog_output (state_t *state)
{
    while ((ch = getchar()) != state->win_keys->back) {
        run_other_win_key (ch, state);
    }
}



void gdb_win_breakpoints(state_t *state)
{
    while ((ch = getchar()) != state->win_keys->back) {
        run_other_win_key (ch, state);
    }
}



void gdb_win_watches (state_t *state)
{
    while ((ch = getchar()) != state->win_keys->back) {
        run_other_win_key (ch, state);
    }
}



void gdb_win_local_vars (state_t *state)
{
    while ((ch = getchar()) != state->win_keys->back) {
        run_other_win_key (ch, state);
    }
}



void gdb_win_prompt (state_t *state)
{
    while ((ch = getchar()) != state->win_keys->back) {
        run_other_win_key (ch, state);
    }
}



void gdb_win_src_file (state_t *state)
{
    while ((ch = getchar()) != state->win_keys->back) {
        run_other_win_key (ch, state);
    }
}



void gdb_win_registers (state_t *state)
{
    while ((ch = getchar()) != state->win_keys->back) {
        run_other_win_key (ch, state);
    }
}



/********************
  Update window data
 ********************/

// CURRENT: How to update window data after commands?
    // update_window_data (code)
        // thoughts
            // commands update prompt, program output
            // updates get program data
        // point curr_plugin to plugin with correct code, path for correct marks
    // clean up debug_state variables

void gdb_update_local_vars (state_t *state)
{
    char *info_locals_cmd = "info locals\n";
    bytes = write (INPUT_PIPE, info_locals_cmd, strlen (info_locals_cmd));
    CHECK_BYTES(bytes);
}







/*****************
  Debugger output
 *****************/



/*
    Insert start mark, plugin output file path into debugger output
    -------

        >START:<output_file_path>:

    - Called in run_plugin()
    - Read, parsed in debugger reader process
*/
void gdb_insert_output_start_marker (state_t *state)
{
    char *start = "echo >START:";
    char *path = state->curr_plugin->data_file_path;

    bytes  = write (INPUT_PIPE, start, strlen (start));
    bytes += write (INPUT_PIPE, path, strlen (path));
    bytes += write (INPUT_PIPE, ":\n", 1);
    CHECK_BYTES(bytes);
}



/*
    Insert end mark, plugin code into debugger output
    -------

        >END:<plugin_code>:

    - Called in run_plugin()
    - Read, parsed in debugger reader process
*/
void gdb_insert_output_end_marker (state_t *state)
{
    char *end = "echo >END:";
    char *code = state->curr_plugin->code;

    bytes  = write (INPUT_PIPE, end, strlen (end));
    bytes += write (INPUT_PIPE, code, strlen (code));
    bytes += write (INPUT_PIPE, ":\n", 1);
    CHECK_BYTES(bytes);
}



/*
    Parse debugger output
    ----------
    - Called in start_debugger_reader_proc()
*/
void gdb_parse_output (int *rstate, char *in_buffer, char *debug_out_path, char *program_out_path, char *code)
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

    *rstate = READER_RECEIVING;
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

            // Markers set in run_plugin(), used in start_debugger_reader_proc()
            //
            //  if  ">START:<path>:" marker  -> set debug out path
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
                *rstate = READER_RECEIVING;
                debug_out_ptr = clear_and_open_file_for_append (debug_out_path);
            } 

            // if  ">END:<code>:" marker  -> set code, set rstate to done
            else if ( *buff_ptr      == '>' && 
                     *(buff_ptr + 1) == 'E' && 
                     *(buff_ptr + 2) == 'N' && 
                     *(buff_ptr + 3) == 'D') {

                buff_ptr += 5;
                i = 0;
                while (*buff_ptr != ':') {
                    code [i++] = *buff_ptr++;
                }
                code [i] = '\0';
                *rstate = READER_DONE;
            }

            // if  ">EXIT" marker  -> set exit rstate
            else if ( *buff_ptr      == '>' && 
                     *(buff_ptr + 1) == 'E' && 
                     *(buff_ptr + 2) == 'X' && 
                     *(buff_ptr + 3) == 'I' && 
                     *(buff_ptr + 4) == 'T') {

                *rstate = READER_EXIT;
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

