
/********************
  Update window data
 ********************/

#include <unistd.h>

#include "../data.h"
#include "../utilities.h"

ssize_t wbytes;



void gdb_update_assembly (state_t *state)
{
    // TODO: fill assembly window
    const char *asm_cmd = "-data-disassemble -s $pc -e '$pc + 1' -- 0\n";
    wbytes = write (INPUT_PIPE, asm_cmd, strlen (asm_cmd));
    CHECK_BYTES(wbytes);
}



void gdb_update_breakpoints (state_t *state)
{
    /*
        TODO:
        -------
        - Parse breakpoints
        - See prototypes/breakpoint.data
    */
    const char *break_cmd = "-break-list\n";
    wbytes = write (INPUT_PIPE, break_cmd, strlen (break_cmd));
    CHECK_BYTES(wbytes);
}



void gdb_update_local_vars (state_t *state)
{
    const char *info_locals_cmd = "info locals\n";
    wbytes = write (INPUT_PIPE, info_locals_cmd, strlen (info_locals_cmd));
    CHECK_BYTES(wbytes);
}



void gdb_update_source_file (state_t *state)
{
    /*
        TODO: 
        -----------
        - parse -stack-list-frames output:
            - "line"       - line number
            - "fullname"   - absolute path to source file
    */
    const char *src_file_cmd = "-stack-list-frames\n";
    wbytes = write (INPUT_PIPE, src_file_cmd, strlen (src_file_cmd));
    CHECK_BYTES(wbytes);
}



void gdb_update_registers (state_t *state)
{
    /*
        TODO:
        -----------
        - parse output:
            -data-list-register-names
            -data-list-register-values
            -data-list-changed-registers    (returns list of indexes as strings)
    */
    const char *asm_cmd = "echo <assembly data>\n";
    wbytes = write (INPUT_PIPE, asm_cmd, strlen (asm_cmd));
    CHECK_BYTES(wbytes);
}



void gdb_update_watchpoints (state_t *state)
{
    /*
        TODO:
        -------
        - Parse watchpoints
        - See prototypes/breakpoint.data
    */
    const char *break_cmd = "-break-list\n";
    wbytes = write (INPUT_PIPE, break_cmd, strlen (break_cmd));
    CHECK_BYTES(wbytes);
}



