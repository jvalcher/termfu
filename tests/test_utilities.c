#include "../src/data.h"

void
print_buffers (state_t *state)
{
    debugger_t *debug = state->debugger;

    printf ("\nCLI:\n%s", debug->cli_buffer);
    printf ("\nDATA:\n%s", debug->data_buffer);
    printf ("\nFORMAT:\n%s", debug->format_buffer);
    printf ("\nASYNC:\n%s", debug->async_buffer);
    printf ("\nPROGR:\n%s", debug->program_buffer);
}

