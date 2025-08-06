/*
    Misc utilities
*/
#ifndef UTILS_H
#define UTILS_H

#define MAX_CONCAT_STRS  20

/**********
  buffer_t
 **********/

typedef struct buffer buffer_t;

#define KB_TO_BYTES(b) ((b) * 1024)

/*
    Create buffer_t object
    ---------
    - Returns: pointer to buffer_t object or NULL
*/
buffer_t* create_buffer(int size);

/*
    Clean up buffer_t object
*/
void destroy_buffer(buffer_t *b);

/*
    Buffer data interface
    --------
    - cp_*() functions erase existing data
    - concat_*() functions concatenate to end of existing data
    - Data wraps around when size exceeded
*/
void clear_buffer(buffer_t *b);
void cp_buf_ch(buffer_t *b, char ch);
void cp_buf_str(buffer_t *b, const char *str);
void concat_buf_ch(buffer_t *b, char ch);
void concat_buf_str(buffer_t *b, const char *str);

/*
    Get buffer data pointer
*/
char* buffer(buffer_t *b);

/******
  Misc
 ******/

/*
    Concatenate strings
    -------
    - Combine maximum of <MAX_CONCAT_STRS> strings
    - Returns: pointer to allocated string or NULL
    - Caller responsible for freeing string
    - Usage:
        char *str = create_str(str1, str2, str3);
*/
char *create_str_impl(int num_strings, ...);
#define create_str(...) create_str_impl(MAX_CONCAT_STRS, __VA_ARGS__, NULL)

#endif
