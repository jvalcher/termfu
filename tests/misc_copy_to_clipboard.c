#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "../src/utilities.h"


//printf "TEST" | xclip -selection clipboard

int
main (void)
{
    char *cmd = "TEST";

    copy_to_clipboard (cmd);

    return 0;
}
