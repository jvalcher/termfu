#include <unistd.h>

#include "data.h"
#include "utilities.h"

#include "parse_cli_arguments.h"

/*
    Parse command-line arguments
*/
void
parse_cli_arguments (int argc,
                     char *argv[],
                     state_t *state)
{
    int opt;
    debugger_t *debugger = state->debugger;

    debugger->curr = UNKNOWN;

    // flags
    while ((opt = getopt(argc, argv, "d:")) != -1) {
        switch (opt) {

            // debugger
            case 'd':
                if (strcmp (optarg, "gdb") == 0) {
                    debugger->curr = DEBUGGER_GDB;
                } else {
                    pfeme ("Unrecognized debugger \"%s\"\n", optarg);
                }
                break;

            default:
                pfeme ("Unrecognized flag \"%d\"\n", opt);
        }
    }

    // program
    if (optind < argc) {
        debugger->prog_path = argv[optind];
    } else {
        pfeme ("Usage: \"termvu -d gdb a.out\"\n");
    }
}


    
