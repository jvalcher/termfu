#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "test_utilities.h"

void
process_pause (void)
{
    printf (
        "\n"
        "Process ID:    \033[0;36m%ld\033[0m \n"
        "\n"
        "Connect to this process with debugger\n"
        "\n"
        "    $ make conn_proc_<debugger>\n"
        "    - Set breakpoint\n"
        "    - Continue\n"
        "\n"
        "Press any key to continue...\n"
        "\n",
        (long) getpid ());

    // wait...
    getchar ();
}


