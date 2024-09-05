#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    char *cmd = "source-highlight -i ../misc/hello.c -s c -f esc -o output";
    int c; // Variable to hold each character
    FILE *fp;

    // Run the command to create the output file
    system(cmd);

    // Open the output file for reading
    fp = fopen("output", "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open output file\n");
        return 1;
    }

    // Read and print each character from the file until EOF
    while ((c = fgetc(fp)) != EOF) {
        putchar(c);
    }

    // Close the file stream
    fclose(fp);

    return 0;
}

