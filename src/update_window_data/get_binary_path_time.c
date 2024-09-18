#include <sys/stat.h>

#include "get_binary_path_time.h"
#include "../data.h"
#include "../insert_output_marker.h"
#include "../utilities.h"
#include "../parse_debugger_output.h"

static void get_binary_path_time_gdb (state_t *state);


void
get_binary_path_time (state_t *state)
{
    switch (state->debugger->index) {
        case DEBUGGER_GDB:
            get_binary_path_time_gdb (state);
            break;
        case DEBUGGER_PDB:
            break;
    }
}



static void
get_binary_path_time_gdb (state_t *state)
{
    int i;
    char *src_ptr,
        *dest_ptr;
    struct stat file_stat;

    const char *path_str = "Symbols from \"";

    i = 0;
    src_ptr  = state->debugger->cli_buffer;
    dest_ptr = state->debugger->prog_path;

    send_command_mp (state, "info file\n");

    if ((src_ptr = strstr (src_ptr, path_str)) != NULL) {

        // path
        src_ptr += strlen (path_str);
        while (*src_ptr != '\"' && i < PROGRAM_PATH_LEN - 1) {
            *dest_ptr++ = *src_ptr++;
            ++i;
        }
        *dest_ptr = '\0';

        // last updated time
        if (stat (state->debugger->prog_path, &file_stat) == 0) {
            state->debugger->prog_update_time = file_stat.st_mtim.tv_sec;
        } else {
            pfeme ("Unable to get status of file \"%s\"\n", state->debugger->prog_path);
        }
    }
}


