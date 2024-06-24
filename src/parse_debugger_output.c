#include <ctype.h>

#include "parse_debugger_output.h"
#include "data.h"
#include "utilities.h"

void gdb_parse_debugger_output (int*, char*, char*, char*, char*);


FILE *debug_out_ptr   = NULL,
     *program_out_ptr = NULL;
bool  parse_first_run = true;

/*
    Parse debugger output
    ----------
    - Called in start_debugger_reader_proc()
*/
void parse_debugger_output (int   debugger, 
                            int  *reader_state, 
                            char *in_buffer, 
                            char *plugin_code,
                            char *debug_out_path, 
                            char *program_out_path)
{
    switch (debugger) {
        case (DEBUGGER_GDB):
            gdb_parse_debugger_output (reader_state, in_buffer, plugin_code, debug_out_path, program_out_path);
            break;
    }
}



/*****************
  Implementations
 *****************/


/*
    GDB
*/
void gdb_parse_debugger_output (int  *reader_state, 
                                char *in_buffer, 
                                char *plugin_code, 
                                char *debug_out_path, 
                                char *program_out_path)
{
    bool  is_gdb_output,
          is_prog_output,
          is_newline;
    char *buff_ptr;
    int   i;

    buff_ptr = in_buffer;

    is_gdb_output = false;
    is_prog_output = false;
    is_newline = true;

    if (parse_first_run) {
        if (debug_out_ptr == NULL) {
            *reader_state = READER_RECEIVING;
            debug_out_ptr   = fopen (debug_out_path, "w");
            program_out_ptr = fopen (program_out_path, "w");
        }
    }

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

            // ==  \\\t,\n, etc. -> skip
            if (*buff_ptr == '\\' && (isalpha(*(buff_ptr + 1)) || *(buff_ptr + 1) == '\n') ) {
                buff_ptr += 2;
            }

            // ==  \\\"  ->  \"
            else if (*buff_ptr == '\\' && *(buff_ptr + 1) == '\"' ) {
                buff_ptr += 1;
                fputc (*buff_ptr++, debug_out_ptr);
            }

            // ==  \"  ->  skip
            else if (*buff_ptr == '\"') {
                buff_ptr += 1;
            }

            // ==  '>'
            else if (*buff_ptr == '>') {

                //  beginning of command output marker
                //
                //      >START:<plugin code>:<output file path>:
                //
                if (*(buff_ptr + 1) == 'S' && 
                    *(buff_ptr + 2) == 'T' && 
                    *(buff_ptr + 3) == 'A' && 
                    *(buff_ptr + 4) == 'R' && 
                    *(buff_ptr + 5) == 'T') {

                    // get plugin code
                    buff_ptr += 7;
                    i = 0;
                    while (*buff_ptr != ':') {
                        plugin_code [i++] = *buff_ptr++;
                    }
                    plugin_code [i] = '\0';

                    // get output path
                    ++buff_ptr;
                    i = 0;
                    while (*buff_ptr != ':') {
                        debug_out_path [i++] = *buff_ptr++;
                    }
                    debug_out_path [i] = '\0';
                    ++buff_ptr;

                    // TODO: erase data window contents
                    // TODO: insert timestamp, delimiter, etc.

                    // open debugger output file
                    debug_out_ptr = fopen (debug_out_path, "a");
                    if (debug_out_ptr == NULL) {
                        pfeme ("Unable to open debugger output file \"%s\"", debug_out_path);
                    }

                    // open program output file  (set in reader process)
                    program_out_ptr = fopen (program_out_path, "a");
                    if (program_out_ptr == NULL) {
                        pfeme ("Unable to open program output file \"%s\"", program_out_path);
                    }

                    *reader_state = READER_RECEIVING;

                    is_newline = true;
                    is_gdb_output = false;
                }

                //  end of command output marker
                //
                //      >END:<plugin code>:
                //
                else if (*(buff_ptr + 1) == 'E' && 
                         *(buff_ptr + 2) == 'N' && 
                         *(buff_ptr + 3) == 'D') {

                    buff_ptr += 4;

                    // close debugger, program output files
                    fclose (debug_out_ptr);
                    fclose (program_out_ptr);

                    *reader_state = READER_DONE;

                    break;
                }

                // exit marker
                //
                //      >EXIT
                //
                else if (*(buff_ptr + 1) == 'E' && 
                         *(buff_ptr + 2) == 'X' && 
                         *(buff_ptr + 3) == 'I' && 
                         *(buff_ptr + 4) == 'T') {

                    *reader_state = READER_EXIT;
                    break;
                }
            } // ==  '>'

            // ==  char
            else {
                if (debug_out_ptr) {
                    fputc (*buff_ptr++, debug_out_ptr);
                } else {
                    pfeme ("Debugger output file not set");
                }
            }

        } // is_gdb_output

        // program output
        else if (is_prog_output) {
            fputc (*buff_ptr++, program_out_ptr);
        }

        else {
            ++buff_ptr;
        }
    }

    if (parse_first_run) {
        if (strstr (in_buffer, "(gdb)")) {
            fclose (debug_out_ptr);
            fclose (program_out_ptr);
            parse_first_run = false;
        }
    }
}

