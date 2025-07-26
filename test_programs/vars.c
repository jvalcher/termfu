#include <stdio.h>
#include <string.h>

// Global variable definitions
int integer_var = 42;
float float_var = 3.14159f;
double double_var = 2.718281828459;
char char_var = 'A';
unsigned long ulong_var = 4294967295;
short short_var = 32767;
long long_var = -9223372036854775807L;
unsigned int uint_var = 1234567890U;
const char* short_string = "Hello, World!";
char long_string[] = "This is a very long string variable that is over 150 characters long to meet the requirements. "
                     "It keeps going and going to make sure we meet the length requirement for this exercise. It should "
                     "have more than 150 characters easily.";
int integer_array[5] = {1, 2, 3, 4, 5};



// Function prototypes
void print_variables();
int calculate_sum(int a, int b);
void modify_string(char* str);

int main() {
    // Local variables
    int result;
    char str_copy[256] = {0};

    // Print all global variables
    print_variables();

    // Perform a calculation with integer variables
    result = calculate_sum(integer_var, short_var);
    printf("Sum of integer_var and short_var: %d\n", result);

    // Copy and modify the long string
    strncpy(str_copy, long_string, sizeof(str_copy) - 1);
    str_copy[sizeof(str_copy) - 1] = '\0';  // Ensure null termination
    modify_string(str_copy);
    printf("Modified long string: %s\n", str_copy);

    // Array operations
    printf("Array elements: ");
    for (int i = 0; i < 5; ++i) {
        printf("%d ", integer_array[i]);
    }
    printf("\n");

    // Pointer operations
    int* ptr = &integer_var;
    printf("Pointer to integer_var points to value: %d\n", *ptr);

    // Mathematical operations with floating-point numbers
    float sum_float = float_var + (float)double_var;
    printf("Sum of float_var and double_var as float: %f\n", sum_float);

    return 0;
}

// Function to print the values of global variables
void print_variables() {
    printf("Integer: %d\n", integer_var);
    printf("Float: %f\n", float_var);
    printf("Double: %lf\n", double_var);
    printf("Char: %c\n", char_var);
    printf("Unsigned long: %lu\n", ulong_var);
    printf("Short: %d\n", short_var);
    printf("Long: %ld\n", long_var);
    printf("Unsigned int: %u\n", uint_var);
    printf("Short string: %s\n", short_string);
    printf("Long string: %s\n", long_string);
}

// Function to calculate the sum of two integers
int calculate_sum(int a, int b) {
    return a + b;
}

// Function to modify a given string
void modify_string(char* str) {
    strcat(str, " - modified");
}
