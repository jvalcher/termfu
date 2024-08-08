
#include <stdio.h>
#include <string.h>



int main() {

    char input[] = "\"^done,line=\\\"2\\\",file=\\\"hello.c\\\",fullname=\\\"/home/jvalcher/Git/term_debug/misc/hello.c\\\",macro-info=\\\"0\\\"\"";
    char *start, *end;
    char fullname[256];  // Adjust the size as needed

    // Find the start of the fullname value
    start = strstr(input, "fullname=\\\"");
    if (start) {
        start += strlen("fullname=\\\"");
        // Find the end of the fullname value
        end = strstr(start, "\\\"");
        if (end) {
            size_t length = end - start;
            strncpy(fullname, start, length);
            fullname[length] = '\0';
            printf("Fullname: %s\n", fullname);
        } else {
            printf("End quote not found.\n");
        }
    } else {
        printf("Fullname not found.\n");
    }

    return 0;
}
