#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "gdb.h"

#define BUF_LEN  4096

int main() {

    FILE *file;
    char in_buffer  [BUF_LEN];
    char out_buffer [BUF_LEN];

    file = fopen("./debug_output.txt", "r");

    memset (out_buffer, '\0', BUF_LEN);

    size_t read_size = fread(in_buffer, sizeof(char), BUF_LEN, file);

    printf("%s\n\n", in_buffer);

    gdb_parse_output (in_buffer, out_buffer);

    printf ("%s\n", out_buffer);

    return 0;
}
