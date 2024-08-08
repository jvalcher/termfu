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
    reader_t reader;

    reader.state = READER_RECEIVING;
    reader.output_line_buffer[0] = '\0';
    reader.cli_buffer_ptr = state->debugger->cli_buffer;
    reader.program_buffer_ptr = state->debugger->program_buffer;
    reader.data_buffer_ptr = state->debugger->data_buffer;
    reader.async_buffer_ptr = state->debugger->async_buffer;

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
            case READER_RECEIVING:
                break;

            case READER_DONE:
#ifdef DEBUG
                printf ("CLI BUFFER:\n %s\n", state->debugger->cli_buffer);
                printf ("PROGRAM BUFFER:\n %s\n", state->debugger->program_buffer);
                printf ("DATA BUFFER:\n %s\n", state->debugger->data_buffer);
#endif

            case READER_EXIT:
                running = false;
                break;
        }
    }
}



void
parse_debugger_output_gdb (reader_t *reader)
{
    bool  is_gdb_output,
          is_prog_output,
          is_data_output,
          is_async_output,
          is_newline;
    char *buff_ptr;

    buff_ptr = reader->output_line_buffer;

    is_gdb_output = false;
    is_prog_output = false;
    is_data_output = false;
    is_async_output = false;
    is_newline = true;

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

            // program output
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
                *reader->cli_buffer_ptr++ = *buff_ptr++;
            }

            // program
            else if (is_prog_output) {
                is_prog_output = false;
                *reader->program_buffer_ptr++ = *buff_ptr++;
            }

            // data
            else if (is_data_output) {
                is_data_output = false;
                *reader->data_buffer_ptr++ = *buff_ptr++;
            }

            // async
            else if (is_async_output) {
                is_async_output = false;
                *reader->async_buffer_ptr++ = *buff_ptr++;
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
                *reader->cli_buffer_ptr++ = *buff_ptr++;
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

                    *reader->program_buffer_ptr = '\0';
                    *reader->cli_buffer_ptr = '\0';
                    *reader->data_buffer_ptr = '\0';

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
                *reader->cli_buffer_ptr++ = *buff_ptr++;
            }
        } // is_gdb_output

        // program output
        else if (is_prog_output) {
            *reader->program_buffer_ptr++ = *buff_ptr++;
        }

        // data output
        else if (is_data_output) {
            *reader->data_buffer_ptr++ = *buff_ptr++;
        }

        // async output
        else if (is_async_output) {
            *reader->async_buffer_ptr++ = *buff_ptr++;
        }

        else {
            ++buff_ptr;
        }
    }
}
