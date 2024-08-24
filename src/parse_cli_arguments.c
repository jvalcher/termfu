#include "data.h"
#include "utilities.h"

#include "parse_cli_arguments.h"

/*
    Parse command-line arguments
*/
void
parse_cli_arguments (int argc,
                     char *argv[],
                     debugger_t *debugger)
{
    // TODO: add flag options
        // -h  - help
        // -d  - choose debugger
        // -f  - config file path (optional)
        // -l  - set initial layout (optional)
    if (argc > 1) {
        debugger->prog_path = argv [1];
    } else {
        pfeme ("Usage:  term_debug a.out\n");
    }
}


    
