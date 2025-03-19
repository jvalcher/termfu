#include <ctype.h>

#include "get_local_vars.h"
#include "../data.h"
#include "../utilities.h"
#include "../error.h"
#include "../plugins.h"


static int get_local_vars_gdb (state_t *state);
static int get_local_vars_pdb (state_t *state);



int
get_local_vars (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            if (get_local_vars_gdb (state) == FAIL)
                pfemr ("Failed to get local variables (GDB)");
            break;
        case (DEBUGGER_PDB):
            if (get_local_vars_pdb (state) == FAIL)
                pfemr ("Failed to get local variables (PDB)");
            break;
    }
    return A_OK;
}



static int
get_local_vars_gdb (state_t *state)
{
    int          index,
                 tmp_index,
                 rem_index;
    window_t    *win;
    char        *src_ptr;
    buff_data_t *dest_data;

    char *key_name  = "name=\"",
         *key_value = "value=\"";

    win       = state->plugins[LcV]->win;
    src_ptr   = state->debugger->data_buffer;
    dest_data = win->buff_data;
    index     = 1;

    if (send_command_mp (state, "-stack-list-locals 1\n") == FAIL)
        pfemr (ERR_DBG_CMD);

    dest_data->buff_pos = 0;
    dest_data->buff[0] = '\0';

    if (strstr (src_ptr, "error") == NULL) {

        while ((src_ptr = strstr (src_ptr, key_name)) != NULL) {

            // index  (used by format_window_data_LcV() to identify start of line when text wrapped)
            cp_wchar (dest_data, '(');
            tmp_index = index;
            do {
                rem_index = tmp_index % 10;
                cp_wchar (dest_data, rem_index + '0');
                tmp_index /= 10;
            } while (tmp_index != 0);
            ++index;
            cp_wchar (dest_data, ')');
            cp_wchar (dest_data, ' ');

            // variable
            src_ptr += strlen (key_name);
            while (*src_ptr != '\"') {
                cp_wchar (dest_data, *src_ptr++);
            }

            cp_wchar (dest_data, ' ');
            cp_wchar (dest_data, '=');
            cp_wchar (dest_data, ' ');

            // value
            src_ptr = strstr (src_ptr, key_value);
            src_ptr += strlen (key_value);

            while ( *src_ptr != '}') {

                // skip string hex address
                if ( *src_ptr      == '0' &&
                    *(src_ptr + 1) == 'x')
                {
                    while (*src_ptr != ' ' && *src_ptr != '\"') {
                        ++src_ptr;
                    }
                }

                //  \\\t, \\\n  ->  \t, \n
                if (*src_ptr == '\\' && isalpha(*(src_ptr + 1))) {
                    if (*(src_ptr + 1) == 'n') {
                        cp_wchar (dest_data, '\\');
                        cp_wchar (dest_data, 'n');
                        src_ptr += 2;
                    } else if (*(src_ptr + 1) == 't') {
                        cp_wchar (dest_data, '\\');
                        cp_wchar (dest_data, 't');
                        src_ptr += 2;
                    } 
                }

                //  \\\"  ->  \"
                else if (*src_ptr == '\\' && *(src_ptr + 1) == '\"' ) {
                    src_ptr += 1;
                    cp_wchar (dest_data, *src_ptr++);
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
                    cp_wchar (dest_data, *src_ptr++);
                }
            }

            // ..., 4, 5 '}'
            if (*(src_ptr + 1) == '\"') {
                cp_wchar (dest_data, '}');
            }

            cp_wchar (dest_data, '\n');
        }
    } 

    else {
        cp_wchar (dest_data, '\0');
    }

    dest_data->changed = true;

    return A_OK;
}



static int
get_local_vars_pdb (state_t *state)
{
    int          open_arrs,
                 index,
                 tmp_index,
                 rem_index;
    window_t    *win;
    char        *src_ptr;
    buff_data_t *dest_data;

    char *name_str = "{'__name__':";

    win       = state->plugins[LcV]->win;
    src_ptr   = state->debugger->program_buffer;
    dest_data = win->buff_data;
    index     = 1;

    if (send_command_mp (state, "locals()\n") == FAIL)
        pfemr (ERR_DBG_CMD);

    dest_data->buff_pos = 0;
    dest_data->buff[0] = '\0';

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

                // index
                // - used by format_window_data_LcV() to identify start
                //   of line when text wrapped
                cp_wchar (dest_data, '(');
                tmp_index = index;
                do {
                    rem_index = tmp_index % 10;
                    cp_wchar (dest_data, rem_index + '0');
                    tmp_index /= 10;
                } while (tmp_index != 0);
                ++index;
                cp_wchar (dest_data, ')');
                cp_wchar (dest_data, ' ');

                // variable
                src_ptr -= 1;      // skip second ' in 'var'
                while (*src_ptr != '\'') {
                    --src_ptr;
                }
                ++src_ptr;      // skip first ' in 'var'
                while (*src_ptr != '\'') {
                    cp_wchar (dest_data, *src_ptr++);
                }

                cp_wchar (dest_data, ' ');
                cp_wchar (dest_data, '=');
                cp_wchar (dest_data, ' ');

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

                    cp_wchar (dest_data, *src_ptr++);
                }

                cp_wchar (dest_data, '\n');
            }

            else {
                ++src_ptr;
            }
        }

        while (dest_data->buff[dest_data->buff_pos] != '}') {
            --dest_data->buff_pos;
        }
        cp_wchar (dest_data, '\0');
    }

skip_LcV_parse:

    dest_data->changed = true;
    dest_data->new_data = false;

    return A_OK;
}

