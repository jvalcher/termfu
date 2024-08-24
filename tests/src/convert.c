#include <stdlib.h>

/*
*/

#include <stdio.h>

// print hex byte
void int_to_hex_byte (int n)
{
    printf ("int to hex byte - %d: %.2x\n", n, n);
}

// char to upper case
char char_to_upper (char c)
{
    printf ("char to upper - %c: %c\n", c, (c - 'a' + 'A'));
}

// char to lower case
char char_to_lower (char c)
{
    printf ("char to lower - %c: %c\n", c, (c + 'a' - 'A'));
}

// char to int
void char_to_int (char c)
{
    printf ("char to int - %c: %d\n", c, c - '0');
}

// int to char
void int_to_char (int n)
{
    printf ("int to char - %d: %c\n", n, n + '0');
}

// int to binary
#define MAX_BIN_ARR 100
void int_to_binary(int num, char *b) {

    int curr;
    int i = 0;
    char tmp;

    // get bits
    while (num > 0) {
        curr = num % 2; 
        *(b+i++) = curr + '0';
        num /= 2;
    }
    *(b+i) = '\0';

    // reverse
    for (int j = 0; j < i; j++) {
        if (j >= i) break;
        tmp = b[i-1];
        b[i-1] = b[j];
        b[j] = tmp;
        i -= 1;
    }
}

// int to string
char *int_to_str (int n) 
{
    // calculate number of chars
    int num = n;
    int num_chars=0;
    do {
        num_chars += 1;
        num /= 10;
    } while (num != 0);
    num_chars += (num_chars - 1) / 3;   // add commas
    num_chars += 1;                     // add '\0'

    // allocate string
    char *str = (char *) malloc (sizeof (char) * num_chars);

    // create string
    int comma_count = 0;
    for (int i = (num_chars - 1); i >= 0; i--) {

        // add '\0'
        if (i == (num_chars - 1)) {
            str [i] = '\0';
        }

        // add comma
        else if (comma_count == 3) {
            str [i] = ',';
            comma_count = 0;
        }

        // add numbers
        else {
            comma_count += 1;
            str [i] = (n % 10) + '0';
            n /= 10;
        }
    }

    return str;
}


int main (void) 
{

    int_to_hex_byte (15);
    char_to_upper ('j');
    char_to_lower ('C');
    char_to_int ('5');
    int_to_char (8);
    int_to_char (18);

    // int to binary
    int n = 1218;
    char bin [MAX_BIN_ARR];
    int_to_binary(n, bin);
    printf("integer: \t%X \nbinary: %s\n", n, bin);

    // int to string
    int n_arr[] = {12345, 857463558, 456};
    for (int i = 0; i < 3; i++) {
        char *str = int_to_str (n_arr [i]);
        printf("%s\n", str);
        free (str);
    }

    return 0;
}

