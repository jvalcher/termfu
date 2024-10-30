#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#include "parse_debugger_output.h"
#include "data.h"
#include "utilities.h"

void parse_debugger_output_gdb (debugger_t*);
void parse_debugger_output_pdb (debugger_t*);



int
parse_debugger_output (state_t *state)
{
    bool running;
    ssize_t bytes_read = 0;

    state->debugger->cli_pos = 0;
    state->debugger->program_pos = 0;
    state->debugger->data_pos = 0;
    state->debugger->async_pos = 0;

    state->debugger->reader_state = READER_RECEIVING;
    running = true;

    while (running) 
    {
        // read debugger stdout
        state->debugger->reader_buffer[0] = '\0';
        bytes_read = read (state->debugger->stdout_pipe, 
                           state->debugger->reader_buffer,
                           READER_BUF_LEN - 1);
        if (bytes_read == -1) {
            pfem ("read error: %s", strerror (errno));
            pemr ("Failed to read debugger stdout");
        }
        state->debugger->reader_buffer [bytes_read] = '\0';

        // parse output
        switch (state->debugger->index) {
            case DEBUGGER_GDB:
                parse_debugger_output_gdb (state->debugger);
                break;
            case DEBUGGER_PDB:
                parse_debugger_output_pdb (state->debugger);
                break;
        }

        // set reader state
        switch (state->debugger->reader_state) {
            case READER_RECEIVING:
                break;
            case READER_DONE:
                running = false;
                break;
        }
    }

    return A_OK;
}



/*
    Parse GDB/MI output
    --------

    "<first line character>" :  <destination buffer>

    "~" :  state->debugger->cli_buffer     (GDB CLI console window output)
    "^" :  state->debugger->data_buffer    (Data values e.g. source file path, line number, breakpoints)
    "*" :  state->debugger->async_buffer   (Async state change info e.g. started, stopped)
    ""  :  state->debugger->program_buffer (Debugged program's CLI output)
*/
void
parse_debugger_output_gdb (debugger_t *debugger)
{
    bool  is_gdb_output,
          is_prog_output,
          is_data_output,
          is_async_output,
          is_newline;
    char *buff_ptr;

    is_gdb_output = false;
    is_prog_output = false;
    is_data_output = false;
    is_async_output = false;
    is_newline = true;

    buff_ptr = debugger->reader_buffer;

    while (*buff_ptr != '\0') {

        // set type of output for new line
        if (is_newline && *buff_ptr != '\n') {

            is_newline = false;

            // cli
            if (*buff_ptr == '~') {
                is_gdb_output = true;
                ++buff_ptr;
            } 

            // data
            else if (*buff_ptr == '^') {
                is_data_output = true;
                ++buff_ptr;
            }

            // async
            else if (*buff_ptr == '*') {
                is_async_output = true;
                ++buff_ptr;
            }

            // program
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
                cp_dchar (debugger, *buff_ptr++, CLI_BUF);
            }

            // program
            else if (is_prog_output) {
                is_prog_output = false;
                cp_dchar (debugger, *buff_ptr++, PROGRAM_BUF);
            }

            // data
            else if (is_data_output) {
                is_data_output = false;
                cp_dchar (debugger, *buff_ptr++, DATA_BUF);
            }

            // async
            else if (is_async_output) {
                is_async_output = false;
                cp_dchar (debugger, *buff_ptr++, ASYNC_BUF);
            }

            else {
                ++buff_ptr;
            }
        }

        // gdb cli output
        else if (is_gdb_output) {

            //  \\\t, \\\n
            if (*buff_ptr == '\\' && isalpha(*(buff_ptr + 1))) {
                cp_dchar (debugger, '\\', CLI_BUF);
                if (*(buff_ptr + 1) == 'n') {
                    cp_dchar (debugger, 'n', CLI_BUF);
                    buff_ptr += 2;
                } else if (*(buff_ptr + 1) == 't') {
                    cp_dchar (debugger, 't', CLI_BUF);
                    buff_ptr += 2;
                } 
            }

            //  \\\"  ->  \"
            else if (*buff_ptr == '\\' && *(buff_ptr + 1) == '\"' ) {
                buff_ptr += 1;
                cp_dchar (debugger, *buff_ptr++, CLI_BUF);
            }

            //  \"  ->  skip
            else if (*buff_ptr == '\"') {
                buff_ptr += 1;
            }

            //  \\\\  ->  skip
            else if (*buff_ptr == '\\' && *(buff_ptr + 1) == '\\' ) {
                buff_ptr += 1;
            }

            //  '>'
            else if (*buff_ptr == '>') {

                //  beginning of command output marker
                //
                //      ">START\n"
                //
                if (*(buff_ptr + 1) == 'S' && 
                    *(buff_ptr + 2) == 'T' && 
                    *(buff_ptr + 3) == 'A' && 
                    *(buff_ptr + 4) == 'R' && 
                    *(buff_ptr + 5) == 'T') {

                    buff_ptr += 7;
                    debugger->reader_state = READER_RECEIVING;
                    is_newline = true;
                    is_gdb_output = false;
                }

                //  end of command output marker
                //
                //     ">END\n"
                //
                else if (*(buff_ptr + 1) == 'E' && 
                         *(buff_ptr + 2) == 'N' && 
                         *(buff_ptr + 3) == 'D') {

                    debugger->reader_state = READER_DONE;
                    break;
                }

                else {
                    cp_dchar (debugger, *buff_ptr++, CLI_BUF);
                }
            }

            else {
                cp_dchar (debugger, *buff_ptr++, CLI_BUF);
            }

        }

        // program output
        else if (is_prog_output) {
            cp_dchar (debugger, *buff_ptr++, PROGRAM_BUF);
        }

        // data output
        else if (is_data_output) {
            cp_dchar (debugger, *buff_ptr++, DATA_BUF);
        }

        // async output
        else if (is_async_output) {
            cp_dchar (debugger, *buff_ptr++, ASYNC_BUF);
        }

        else {
            ++buff_ptr;
        }
    }
}



/*
    Parse PDB output
    -------

    ->, >, ***,
    debug strings  -> cli_buffer

    Abc            -> program buffer
*/
void
parse_debugger_output_pdb (debugger_t *debugger)
{
    char *break_str    = "Breakpoint 1 at /",
         *restart_str  = "Restarting /",
         *return_str   = "--Return--",
         *call_str     = "--Call--",
         *where_s_str  = "  /",
         *where_c_str  = "  <",
         *finished_str = "The program finished and will be restarted";

    char *buff_ptr = debugger->reader_buffer;

    while (*buff_ptr != '\0') {

        // newline
        if (*(buff_ptr - 1) == '\n' || buff_ptr == debugger->reader_buffer) {

            // ->
            if ( *buff_ptr      == '-' &&
                *(buff_ptr + 1) == '>' &&
                *(buff_ptr + 2) == ' ')
            {
                do {
                    cp_dchar (debugger, *buff_ptr++, CLI_BUF);
                } while (*(buff_ptr - 1) != '\n');
            }

            // >
            else if (*buff_ptr == '>') {

                // > /path/...
                if (*(buff_ptr + 1) == ' ' &&
                    (*(buff_ptr + 2) == '/' || *(buff_ptr + 2) == '<'))
                do {
                    cp_dchar (debugger, *buff_ptr++, CLI_BUF);
                } while (*(buff_ptr - 1) != '\n' && *buff_ptr != '\0');

                // program output
                else {
                    do {
                        cp_dchar (debugger, *buff_ptr++, PROGRAM_BUF);
                    } while (*(buff_ptr - 1) != '\n' && *buff_ptr != '\0');
                }
            }

            // *** msg
            else if ( *buff_ptr      == '*' &&
                     *(buff_ptr + 1) == '*' &&
                     *(buff_ptr + 2) == '*')
            {
                do {
                    cp_dchar (debugger, *buff_ptr++, CLI_BUF);
                } while (*(buff_ptr - 1) != '\n' && *buff_ptr != '\0');
            }

            // skip prompt
            else if ( *buff_ptr      == '('  &&
                     *(buff_ptr + 1) == 'P'  &&
                     *(buff_ptr + 2) == 'd'  &&
                     *(buff_ptr + 3) == 'b'  &&
                     *(buff_ptr + 4) == ')')
                {

                buff_ptr  += 6;

                if (*buff_ptr == '\'') {

                    // start output
                    if (*(buff_ptr + 1) == '>' && 
                        *(buff_ptr + 2) == 'S' && 
                        *(buff_ptr + 3) == 'T' && 
                        *(buff_ptr + 4) == 'A' && 
                        *(buff_ptr + 5) == 'R' && 
                        *(buff_ptr + 6) == 'T')
                    {
                        buff_ptr += 8;
                        debugger->reader_state = READER_RECEIVING;
                    }

                    // end output
                    else if (*(buff_ptr + 1) == '>' && 
                             *(buff_ptr + 2) == 'E' && 
                             *(buff_ptr + 3) == 'N' && 
                             *(buff_ptr + 4) == 'D')
                    {
                        debugger->reader_state = READER_DONE;
                        break;
                    }
                }
            }

            // read() sometimes puts this as first line instead of following (pdb) prompt (see above)
            else if (*buff_ptr == '\'') {

                // start output
                if (*(buff_ptr + 1) == '>' && 
                    *(buff_ptr + 2) == 'S' && 
                    *(buff_ptr + 3) == 'T' && 
                    *(buff_ptr + 4) == 'A' && 
                    *(buff_ptr + 5) == 'R' && 
                    *(buff_ptr + 6) == 'T')
                {
                    buff_ptr += 8;
                    debugger->reader_state = READER_RECEIVING;
                }

                // end output
                else if (*(buff_ptr + 1) == '>' && 
                         *(buff_ptr + 2) == 'E' && 
                         *(buff_ptr + 3) == 'N' && 
                         *(buff_ptr + 4) == 'D')
                {
                    debugger->reader_state = READER_DONE;
                    break;
                }
            }

            // misc cli strings
            else if (strncmp (buff_ptr, break_str, strlen (break_str)) == 0 || 
                     strncmp (buff_ptr, return_str, strlen (return_str)) == 0 || 
                     strncmp (buff_ptr, finished_str, strlen (finished_str)) == 0 || 
                     strncmp (buff_ptr, where_s_str, strlen (where_s_str)) == 0 || 
                     strncmp (buff_ptr, where_c_str, strlen (where_c_str)) == 0 || 
                     strncmp (buff_ptr, call_str, strlen (call_str)) == 0 || 
                     strncmp (buff_ptr, restart_str, strlen (restart_str)) == 0)
            {
                do {
                    cp_dchar (debugger, *buff_ptr++, CLI_BUF);
                } while (*(buff_ptr - 1) != '\n' && *buff_ptr != '\0');
            }

            // program output
            else {
                do {
                    cp_dchar (debugger, *buff_ptr++, PROGRAM_BUF);
                } while (*(buff_ptr - 1) != '\n' && *buff_ptr != '\0');
            }
        }

        // sometimes "(pdb)" inline
        else if ( *buff_ptr      == '('  &&
                 *(buff_ptr + 1) == 'P'  &&
                 *(buff_ptr + 2) == 'd'  &&
                 *(buff_ptr + 3) == 'b'  &&
                 *(buff_ptr + 4) == ')')
        {
            buff_ptr    += 6;
            *buff_ptr++ = '\n';
        }

        else {
            cp_dchar (debugger, *buff_ptr++, CLI_BUF);
        }
    }
}

