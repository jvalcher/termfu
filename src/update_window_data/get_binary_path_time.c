#include <string.h>
#include <sys/stat.h>

#include "get_binary_path_time.h"
#include "../utilities.h"
#include "../error.h"
#include "../debugger.h"

static int get_binary_path_time_gdb (void);



int
get_binary_path_time (void)
{
    switch (get_debugger_index()) {
        case DEBUGGER_GDB:
            if (get_binary_path_time_gdb() == FAIL)
                pfemr ("Failed to get binary path and update time (GDB)");
            break;
        case DEBUGGER_PDB:
            break;
    }
    return A_OK;
}



static int
get_binary_path_time_gdb (void)
{
    int   i, prog_path_len;
    char *src_ptr,
         *dest_ptr;
    struct stat file_stat;

    const char *path_str = "Symbols from \"";
    prog_path_len = get_prog_path_len();

    src_ptr  = get_cli_buffer();
    dest_ptr = get_prog_path();

    if (send_command_mp ("info file\n") == FAIL)
        pfemr (ERR_DBG_CMD);

    if ((src_ptr = strstr (src_ptr, path_str)) != NULL) {

        // path
        i = 0;
        src_ptr += strlen (path_str);
        while (*src_ptr != '\"' && i < prog_path_len - 1) {
            *dest_ptr++ = *src_ptr++;
            ++i;
        }
        *dest_ptr = '\0';

        // last updated time
        dest_ptr = get_prog_path();
        if (stat (dest_ptr, &file_stat) == -1) {
            pfemr_errno ("Failed to get status of file \"%s\"", dest_ptr);
        }
        set_prog_update_time (file_stat.st_mtim.tv_sec);
    }

    return A_OK;
}


