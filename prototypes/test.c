#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>


int main (void)
{
    char share_path[130];
    struct stat st = {0};
    char *home = getenv ("HOME");
    snprintf (share_path, sizeof (share_path), "%s/%s", home, ".local/share/termide");
    if (stat (share_path, &st) == -1) {
        mkdir (share_path, 0700);
    }

     
    return 0;
}
