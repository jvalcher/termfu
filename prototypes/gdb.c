#include <stdbool.h>


/* debug_state_t
    ------
    debugger          - debugger macro identifier
    input_pipe        - debugger input pipe
    src_file_changed  - source file has changed
*/
    //
#define DEBUGGER_UNKNOWN   0
#define DEBUGGER_GDB       1
    //
typedef struct debug_state {

    int    debugger;
    int    input_pipe;
    int    output_pipe;
    char   src_path [256];
    bool   src_file_changed;

} debug_state_t;


// layout


// gdb commands
char *gdb_confirm_off = "-gdb-set confirm off\n",
     *gdb_continue    = "-exec-continue\n",
     *gdb_next        = "-exec-next\n",
     *gdb_step        = "-exec-step\n",
     *gdb_quit        = "-gdb-exit\n",
     *gdb_run         = "-exec-run\n",
     *gdb_break       = "-break-insert",
     *gdb_quit_string = "^exit",
     *gdb_break,
     *gdb_file;

