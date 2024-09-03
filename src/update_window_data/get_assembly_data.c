#include <string.h>

#include "get_assembly_data.h"
#include "../data.h"
#include "../insert_output_marker.h"
#include "../parse_debugger_output.h"
#include "../utilities.h"
#include "../plugins.h"

#define OFFSET_COLS 4


static void get_assembly_data_gdb (state_t *state);



void
get_assembly_data (state_t *state)
{
    switch (state->debugger->curr) {
        case (DEBUGGER_GDB):
            get_assembly_data_gdb (state);
            break;
    }
}


static void
get_assembly_data_gdb (state_t *state)
{
    const char *key_address = "address=\"",
               *key_offset  = "offset=\"",
               *key_inst    = "inst=\"";
    char *src_ptr,
         *dest_ptr,
         *cmd;
    int cols;

    src_ptr  = state->debugger->data_buffer;
    dest_ptr = state->plugins[Asm]->win->buff_data->buff;

    // send debugger command
    cmd = concatenate_strings (3, "-data-disassemble -f ",
                                  state->plugins[Src]->win->file_data->path,
                                  " -l 1 -- 0\n");
    insert_output_start_marker (state);
    send_command (state, cmd);
    insert_output_end_marker (state);
    parse_debugger_output (state);
    free (cmd);

    // create buffer
    if (strstr (src_ptr, "error") == NULL) {

        state->plugins[Asm]->win->buff_data->buff_pos = 0;

        // address
        while ((src_ptr = strstr (src_ptr, key_address)) != NULL) {
            src_ptr += strlen (key_address);
            while (*src_ptr != '\"') {
                *dest_ptr++ = *src_ptr++;
            }

            *dest_ptr++ = ' ';
            *dest_ptr++ = ' ';

            // offset
            cols = OFFSET_COLS;
            src_ptr = strstr (src_ptr, key_offset);
            src_ptr += strlen (key_offset);
            while (*src_ptr != '\"') {
                *dest_ptr++ = *src_ptr++;
                --cols;
            }

            for (int i = 0; i < cols; i++) {
                *dest_ptr++ = ' ';
            }
            *dest_ptr++ = ' ';
            *dest_ptr++ = ' ';

            // get command
            src_ptr = strstr (src_ptr, key_inst);
            src_ptr += strlen (key_inst);
            while (*src_ptr != '\"') {
                *dest_ptr++ = *src_ptr++;
            }

            *dest_ptr++ = '\n';
        }
        *dest_ptr = '\0';

        state->plugins[Asm]->win->buff_data->changed = true;
    }
}




