#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../src/utilities.h"



int
main (void)
{
    struct stat file_stat;
    char *path = "file.txt";
    FILE *fp;
    time_t file_time;

    putchar ('\n');

    // no updates
    fp = fopen (path, "w");
    fprintf (fp, "Initial write\n");
    fclose (fp);

    if (stat (path, &file_stat) != 0) {
        fprintf (stderr, "Unable to get status of file \"%s\"\n", path);
    }
    file_time = file_stat.st_mtim.tv_sec;

    if (file_was_updated (file_time, path)) {
        printf ("No update error\n");
    } else {
        printf ("No update success\n");
    }

    // update
    usleep (2000000);
    fp = fopen (path, "a");
    fprintf (fp, "Initial write\n");
    fclose (fp);

    if (file_was_updated (file_time, path)) {
        printf ("Update success\n");
    } else {
        printf ("Update error\n");
    }

    remove (path);

    return 0;
}
