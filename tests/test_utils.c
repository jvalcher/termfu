#include "../src/utils.h"
#include "test.h"

int passed = 0, failed = 0;

void run_buffer_test(void)
{
    buffer_t *b = create_buffer(10);

    // Copy: string fits in buffer
    const char *str1 = "0123456789";
    cp_buf_str(b, str1);
    test_strs_eq(get_buf(b), str1, "\"%s\" != \"%s\"", get_buf(b), str1);

    // Concat: overflow
    const char *str2 = "01234";
    concat_buf_str(b, str2);
    test_strs_eq(get_buf(b), str2, "\"%s\" != \"%s\"", get_buf(b), str2);

    // Copy: bigger than buffer
    const char *str3 = "0123456789321";
    cp_buf_str(b, str3);
    test_strs_eq(get_buf(b), "321", "\"%s\" != \"%s\"", get_buf(b), "321");

    destroy_buffer(b);
}

int main(void)
{
    test_init();

    run_buffer_test();

    test_results();

    return 0;
}
