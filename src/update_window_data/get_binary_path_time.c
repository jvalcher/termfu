#include <sys/stat.h>

#include "get_binary_path_time.h"
#include "../data.h"
#include "../utilities.h"

static int get_binary_path_time_gdb (state_t *state);


int
get_binary_path_time (state_t *state)
{
    switch (state->debugger->index) {
        case DEBUGGER_GDB:
            if (get_binary_path_time_gdb (state) == RET_FAIL) {
                pfemr ("Failed to get binary path and update time (GDB)");
            }
            break;
        case DEBUGGER_PDB:
            break;
    }

    return RET_OK;
}



static int
get_binary_path_time_gdb (state_t *state)
{
    int i;
    char *src_ptr,
        *dest_ptr;
    struct stat file_stat;

    const char *path_str = "Symbols from \"";

    src_ptr  = state->debugger->cli_buffer;
    dest_ptr = state->debugger->prog_path;

    if (send_command_mp (state, "info file\n") == RET_FAIL) {
        pfemr ("Failed to send debugger command");
    }

    if ((src_ptr = strstr (src_ptr, path_str)) != NULL) {

        // path
        i = 0;
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
            pfemr ("Unable to get status of file \"%s\"\n", state->debugger->prog_path);
        }
    }

    state->debugger->cli_buffer[0]  = '\0';

    return RET_OK;
}


