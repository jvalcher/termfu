#include <string.h>
#include <sys/stat.h>

#include "get_binary_path_time.h"
#include "../utilities.h"
#include "../error.h"

static int get_binary_path_time_gdb (state_t *state);



int
get_binary_path_time (state_t *state)
{
    switch (state->debugger->index) {
        case DEBUGGER_GDB:
            if (get_binary_path_time_gdb (state) == FAIL)
                pfemr ("Failed to get binary path and update time (GDB)");
            break;
        case DEBUGGER_PDB:
            break;
    }
    return A_OK;
}



static int
get_binary_path_time_gdb (state_t *state)
{
    int   i;
    char *src_ptr,
         *dest_ptr;
    struct stat file_stat;

    const char *path_str = "Symbols from \"";

    src_ptr  = state->debugger->cli_buffer;
    dest_ptr = state->debugger->prog_path;

    if (send_command_mp (state, "info file\n") == FAIL)
        pfemr (ERR_DBG_CMD);

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
        if (stat (state->debugger->prog_path, &file_stat) == -1) {
            pfemr_errno ("Failed to get status of file \"%s\"", state->debugger->prog_path);
        }
        state->debugger->prog_update_time = file_stat.st_mtim.tv_sec;
    }

    return A_OK;
}


