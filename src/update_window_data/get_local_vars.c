#include <ctype.h>

#include "get_local_vars.h"
#include "../data.h"
#include "../utilities.h"
#include "../plugins.h"


static void get_local_vars_gdb (state_t *state);
static void get_local_vars_pdb (state_t *state);



void
get_local_vars (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            get_local_vars_gdb (state);
            break;
        case (DEBUGGER_PDB):
            get_local_vars_pdb (state);
            break;
    }
}



static void
get_local_vars_gdb (state_t *state)
{
    window_t *win;
    const
    char *key_name  = "name=\"",
         *key_value = "value=\"";
    char *src_ptr;
    buff_data_t *dest_buff;

    win       = state->plugins[LcV]->win;
    src_ptr   = state->debugger->data_buffer;
    dest_buff = win->buff_data;

    send_command_mp (state, "-stack-list-locals 1\n");

    dest_buff->buff_pos = 0;

    if (strstr (src_ptr, "error") == NULL) {

        // variable
        while ((src_ptr = strstr (src_ptr, key_name)) != NULL) {
            src_ptr += strlen (key_name);
            while (*src_ptr != '\"') {
                cp_char (dest_buff, *src_ptr++);
            }

            cp_char (dest_buff, ' ');
            cp_char (dest_buff, '=');
            cp_char (dest_buff, ' ');

            // value
            src_ptr = strstr (src_ptr, key_value);
            src_ptr += strlen (key_value);
            while ( *src_ptr != '}' ) {

                //  \\\t, \\\n  ->  \t, \n
                if (*src_ptr == '\\' && isalpha(*(src_ptr + 1))) {
                    if (*(src_ptr + 1) == 'n') {
                        cp_char (dest_buff, '\\');
                        cp_char (dest_buff, 'n');
                        src_ptr += 2;
                    } else if (*(src_ptr + 1) == 't') {
                        cp_char (dest_buff, '\\');
                        cp_char (dest_buff, 't');
                        src_ptr += 2;
                    } 
                }

                //  \\\"  ->  \"
                else if (*src_ptr == '\\' && *(src_ptr + 1) == '\"' ) {
                    src_ptr += 1;
                    cp_char (dest_buff, *src_ptr++);
                }

                //  \\\\  ->  '\\'
                else if (*src_ptr == '\\' && *(src_ptr + 1) == '\\' ) {
                    src_ptr += 1;
                }

                //  \"  ->  _
                else if (*src_ptr == '\"') {
                    src_ptr += 1;
                }

                else {
                    cp_char (dest_buff, *src_ptr++);
                }
            }

            // ..., 4, 5 '}'
            if (*(src_ptr + 1) == '\"') {
                cp_char (dest_buff, '}');
            }

            cp_char (dest_buff, '\n');
        }
    } 

    else {
        cp_char (dest_buff, '\0');
    }

    dest_buff->changed = true;
    state->debugger->data_buffer[0]  = '\0';
}



static void
get_local_vars_pdb (state_t *state)
{
    int          open_arrs;
    window_t    *win;
    char        *src_ptr;
    buff_data_t *dest_data;

    char *name_str = "{'__name__':";

    win       = state->plugins[LcV]->win;
    src_ptr   = state->debugger->program_buffer;
    dest_data = win->buff_data;

    send_command_mp (state, "locals()\n");

    dest_data->buff_pos = 0;

    // skip __name__ ... output
    if (strncmp (src_ptr, name_str, strlen (name_str)) == 0) {
        goto skip_LcV_parse;
    }

    // if local variables
    if (*src_ptr == '{' && *(src_ptr + 1) != '}') {

        // skip '{'
        ++src_ptr;

        while (*src_ptr != '\0') {

            // locate ':'
            if (*src_ptr == '\'' && *(src_ptr + 1) == ':') {

                // variable
                src_ptr -= 1;      // skip second ' in 'var'
                while (*src_ptr != '\'') {
                    --src_ptr;
                }
                ++src_ptr;      // skip first ' in 'var'
                while (*src_ptr != '\'') {
                    cp_char (dest_data, *src_ptr++);
                }

                cp_char (dest_data, ' ');
                cp_char (dest_data, '=');
                cp_char (dest_data, ' ');

                // value
                open_arrs = 0;
                src_ptr += 3;  // skip ": "
                while (*src_ptr != '\0') {

                    if (*src_ptr == '[' || *src_ptr == '{') {
                        ++open_arrs;
                    }

                    else if (*src_ptr == ']' || *src_ptr == '}') {
                        --open_arrs;
                    }

                    else if ((*src_ptr      ==  ',' &&
                             *(src_ptr + 1) ==  ' ' &&
                             *(src_ptr + 2) == '\'')
                             && 
                             open_arrs == 0)
                    {
                        break;
                    }

                    cp_char (dest_data, *src_ptr++);
                }

                cp_char (dest_data, '\n');
            }

            else {
                ++src_ptr;
            }
        }

        while (dest_data->buff[dest_data->buff_pos] != '}') {
            --dest_data->buff_pos;
        }
        cp_char (dest_data, '\0');
    }

    // no local variables
    else {
skip_LcV_parse:
        cp_char (dest_data, '\0');
    }

    dest_data->changed = true;
    state->debugger->program_buffer[0] = '\0';
    dest_data->new_data = false;
}

