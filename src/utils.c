#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "utils.h"
#include "error.h"

typedef struct buffer {
    char *buf;   // buffer
    int   size;  // total size
    int   pos;   // null terminator index, string length
} buffer_t;

buffer_t* create_buffer(int size)
{
    buffer_t *b = NULL;
    if (!(b = malloc(sizeof(buffer_t))))
        pfemn_errno("Failed to allocate buffer_t");

    if (!(b->buf = malloc (size + 1)))
        pfemn_errno("Failed to allocate new buffer (size=%d)", size);
    b->buf[0] = '\0';
    b->size = size;
    b->pos = 0;

    return b;
}

void destroy_buffer(buffer_t *b)
{
    if (b) {
        if (b->buf)
            free(b->buf);
        free(b);
    }
}

void clear_buffer(buffer_t *b)
{
    b->pos = 0;
    b->buf[0] = '\0';
}

void concat_buf_ch(buffer_t *b, char ch)
{
    b->buf[b->pos] = ch;
    b->buf[b->pos + 1] = '\0';

    if (b->pos < b->size - 1) {
        b->pos += 1;
    } else {
        b->pos = 0;
    }
}

void concat_buf_str(buffer_t *b, const char *str)
{
    const char *ptr = str;

    for (;;) {

        if (*ptr == '\0') {
            b->buf[b->pos] = '\0';
            break;
        }

        if (b->pos == b->size)
            b->pos = 0;

        b->buf[b->pos++] = *ptr++;
    }
}

void cp_buf_ch(buffer_t *b, char ch)
{
    b->pos = 0;
    concat_buf_ch(b, ch);
}

void cp_buf_str(buffer_t *b, const char *str)
{
    b->pos = 0;
    concat_buf_str(b, str);
}

char* buffer(buffer_t *b)
{
    return b->buf;
}

char* create_str_impl(int max_strs, ...)
{
    int str_len,
        str_count;
    char *sub_str,
         *str;
    va_list strs;
    
    // calculate total string length
    str_len = 0;
    str_count = 0;
    va_start(strs, max_strs);
    for (str = va_arg(strs, char*);
         str != NULL;
         str = va_arg(strs, char*))
    {
        str_len += strlen(str);
        if (++str_count >= max_strs)
            pfemn ("Max strings exceeded");
    }
    va_end (strs);

    // allocate
    if ((str = (char*) malloc(str_len + 1)) == NULL)
        pfemn_errno("Failed to allocate space for string (length: %d)", str_len);
    str [0] = '\0';

    // create string
    va_start (strs, max_strs);
    for (sub_str = va_arg(strs, char*);
         sub_str != NULL;
         sub_str = va_arg(strs, char*))
    {
        strncat (str, sub_str, str_len - strlen(str));
    }
    va_end (strs);

    return str;
}
