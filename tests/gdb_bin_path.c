#include <stdlib.h>

#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/start_debugger.h"
#include "../src/update_window_data/get_binary_path_time.h"


int
main (void)
{
    state_t *state = (state_t*) malloc (sizeof (state_t));
    set_num_plugins (state);
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->debugger->index = DEBUGGER_GDB;

    char *cmd[] = {"gdb", "--quiet", "--interpreter=mi", "../misc/hello"};
    state->command = cmd;

    start_debugger (state);

    get_binary_path_time (state);

    printf ("\n%s\n", state->debugger->prog_path);

    return 0;
}
