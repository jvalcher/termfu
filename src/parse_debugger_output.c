#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "parse_debugger_output.h"
#include "data.h"

void parse_debugger_output_gdb (reader_t *reader);


void 
parse_debugger_output (state_t *state)
{
    bool running = true;
    int bytes_read = 0;
    char *debugger_buffer,
         *program_buffer;
    reader_t reader;

    debugger_buffer = state->debugger->debugger_buffer;
    program_buffer = state->debugger->program_buffer;

    reader.state = READER_RECEIVING;
    reader.output_line_buffer[0] = '\0';
    reader.debugger_buffer_ptr = debugger_buffer;
    reader.program_buffer_ptr = program_buffer;

    while (running) 
    {
        memset (reader.output_line_buffer, '\0', sizeof (reader.output_line_buffer));

        // read debugger stdout
        bytes_read = read (state->debugger->stdout_pipe, 
                           reader.output_line_buffer,
                           sizeof (reader.output_line_buffer) - 1);
        reader.output_line_buffer [bytes_read] = '\0';

        // parse output
        switch (state->debugger->curr) {
            case DEBUGGER_GDB: parse_debugger_output_gdb (&reader); break;
        }

        switch (reader.state) {
            case READER_RECEIVING: break;
            case READER_DONE:      running = false; break;
        }
    }
}



void
parse_debugger_output_gdb (reader_t *reader)
{
    bool  is_gdb_output,
          is_prog_output,
          is_newline;
    char *buff_ptr;

    buff_ptr = reader->output_line_buffer;

    is_gdb_output = false;
    is_prog_output = false;
    is_newline = true;

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
                *reader->debugger_buffer_ptr++ = *buff_ptr++;
            }

            // program
            else if (is_prog_output) {
                is_prog_output = false;
                *reader->program_buffer_ptr++ = *buff_ptr++;
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
                *reader->debugger_buffer_ptr++ = *buff_ptr++;
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

                    reader->state = READER_RECEIVING;

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

                    reader->state = READER_DONE;

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

                    reader->state = READER_EXIT;
                    break;
                }
            } // ==  '>'

            // == char
            else {
                *reader->debugger_buffer_ptr++ = *buff_ptr++;
            }
        } // is_gdb_output

        // program output
        else if (is_prog_output) {
            *reader->program_buffer_ptr++ = *buff_ptr++;
        }

        else {
            ++buff_ptr;
        }
    }
}
