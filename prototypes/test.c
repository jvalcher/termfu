
#include <stdio.h>

char *gdb_cmd[] = {"gdb", "--quiet", "--interpreter=mi", NULL, NULL};

int main (void)
{
    char **cmd;
    char *str = "./hello";

    int num_cmd_strs = sizeof(gdb_cmd) / sizeof(gdb_cmd[0]);
    //printf ("%lu // %lu = %d\n", sizeof(gdb_cmd), sizeof(gdb_cmd[0]), num_cmd_strs);

    cmd = gdb_cmd;
    cmd[3] = str;

    for (int i = 0; i < num_cmd_strs; i++) {
        printf ("%s ", cmd[i]);
    }
}
