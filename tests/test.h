/*
    -----------------
     Testing library
    -----------------
    Author: Jeff Valcher
    URL:    https://github.com/jvalcher/utilities


    - Single header
    - Formatted info messages
    - Exits on failed test
    - Custom test failure messages

            test_strs_eq (str1, str2 "\'%s\' is not equal to \'%s\'", str1, str2);
            test_strs_eq (str1, str2, " ");     // single space for no message

    - Variables

        int passed, failed  - Initialize globally or locally

    - Functions
        
        test_init       - Set passed, failed variables to 0
                        - Print test header with current file and function

        test_cond       - Conditional statement
        test_strs_eq    - Strings are equal
        test_strs_neq   - Strings are not equal

        test_results    - Print test results

    - Example usage:

        ------- tests.c -------

        #include "tests.h"
        #include "unit_test.h"

        void run_tests_01 ()
        {
            // ...

            test_init();

            test_cond     (x > 5, "Test failed: \'%d\' is not greater than 5", x);
            test_strs_eq  (str1, str2, "\'%s\' not equal to \'%s\'", str1, str2);
            test_strs_neq (str2, str3, " ");

            test_results();
        }

        ------- main.c -------

        #include "tests.h"

        int passed = 0,         // Initialize globally or inside individual test functions
            failed = 0;

        int main ()
        {
            run_tests_01();
            return 0;
        }

    - Output:

        ---------------------
        ./tests.c :: run_tests_01()
        ---------------------
            FAIL  (x > 5) Line 25  Test failed: '4' is not greater than 5
            FAIL  (str1 != str2)  Line 27
        Passed: 1,  Failed: 2
*/
#ifndef TEST_H
#define TEST_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define R         "\033[1;0m"         // reset to default
#define RED       "\033[1;31m"
#define CYAN      "\033[1;36m"
#define GREEN     "\033[1;32m"
#define YELLOW    "\033[1;33m"
#define PURPLE    "\033[1;35m"
#define TEST_DIV  "---------------------\n"

extern int passed;
extern int failed;

/*
    Reset statistics; print function and file header
    --------
    - Run at start of every test function
*/
#define test_init() do { \
    passed = 0; \
    failed = 0; \
    printf (TEST_DIV CYAN "%s" R " :: " PURPLE "%s" R "()\n" TEST_DIV, \
            __FILE__, __func__ \
    ); \
} while (0)

/*
    Print test statistics
    --------
    - Run at the end of every test function
*/
#define test_results() do { \
    printf (\
        "Passed: " GREEN "%d" R ",  Failed: " RED "%d\n" R "\n", \
        passed, failed \
    ); \
} while (0)

/*
    Print test failed message
    -------
    - Used by test functions below
    - Output:
        - Condition tested
        - Line number
        - Custom error message
    - Printed between test_init() and test_results() output
*/
#define print_test_failed(...) do { \
    printf (CYAN "  Line " YELLOW "%d" R "  ", __LINE__); \
    printf (__VA_ARGS__); \
    printf ("\n"); \
} while (0)

/****************
  Test functions
 ****************/

#define TEST_FAIL printf("    " RED "FAIL" R "  ")

/*
    Test conditional statement
*/
#define test_cond(condition, ...) do { \
    if (!(condition)) { \
        TEST_FAIL; \
        printf("(%s)", #condition); \
        print_test_failed(__VA_ARGS__); \
        ++failed; \
        test_results(); \
        exit(0); \
    } else { \
        ++passed; \
    } \
} while (0)

/*
    Test if strings equal
*/
#define test_strs_eq(str1, str2, ...) do { \
    if (strcmp((str1), (str2)) != 0) { \
        TEST_FAIL; \
        printf("(%s == %s)", #str1, #str2); \
        print_test_failed(__VA_ARGS__); \
        ++failed; \
        test_results(); \
        exit(0); \
    } else { \
        ++passed; \
    } \
} while (0)

/*
    Test if strings not equal
*/
#define test_strs_neq(str1, str2, ...) do { \
    if (strcmp((str1), (str2)) == 0) { \
        TEST_FAIL; \
        printf("(%s != %s)", #str1, #str2); \
        print_test_failed(__VA_ARGS__); \
        ++failed; \
        test_results(); \
        exit(0); \
    } else { \
        ++passed; \
    } \
} while (0)

#endif
