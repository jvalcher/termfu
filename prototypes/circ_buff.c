
#include <string.h>

#include "../src/data.h"

#define SRC_BUFF_LEN   15
#define DEST_BUFF_LEN  10


/*
    Copy character from source to destination buff_data_t->buff (circular buffer)
*/
#define cp_char(dest_buff_data, src_buff_data) do {\
    dest_buff_data->buff[dest_buff_data->buff_pos] = src_buff_data->buff[src_buff_data->buff_pos++];\
    dest_buff_data->buff[dest_buff_data->buff_pos + 1] = '\0';\
    if (dest_buff_data->buff_pos < dest_buff_data->buff_len - 2) {\
        ++dest_buff_data->buff_pos;\
    } else {\
        dest_buff_data->buff_pos = 0;\
    }\
} while (0)




int
main (void)
{
    buff_data_t *src_buff_data  = (buff_data_t*) malloc (sizeof (buff_data_t));
    buff_data_t *dest_buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
    src_buff_data->buff         = (char*) malloc (SRC_BUFF_LEN);
    dest_buff_data->buff        = (char*) malloc (DEST_BUFF_LEN);
    src_buff_data->buff_len     = SRC_BUFF_LEN;
    dest_buff_data->buff_len    = DEST_BUFF_LEN;


    // source > destination
    strcpy (src_buff_data->buff, "abcdefghijklmn");
    dest_buff_data->buff_pos    = 0;


    src_buff_data->buff_pos     = 0;
    do {
        cp_char (dest_buff_data, src_buff_data);
        printf ("%s\n", dest_buff_data->buff);
    } while (src_buff_data->buff[src_buff_data->buff_pos] != '\0');

    
    puts ("");


    // destination > source
    strcpy (src_buff_data->buff, "abcde");
    dest_buff_data->buff_pos = 0;


    src_buff_data->buff_pos  = 0;
    do {
        cp_char (dest_buff_data, src_buff_data);
        printf ("%s\n", dest_buff_data->buff);
    } while (src_buff_data->buff[src_buff_data->buff_pos] != '\0');


    src_buff_data->buff_pos = 0;
    do {
        cp_char (dest_buff_data, src_buff_data);
        printf ("%s\n", dest_buff_data->buff);
    } while (src_buff_data->buff[src_buff_data->buff_pos] != '\0');


    src_buff_data->buff_pos = 0;
    do {
        cp_char (dest_buff_data, src_buff_data);
        printf ("%s\n", dest_buff_data->buff);
    } while (src_buff_data->buff[src_buff_data->buff_pos] != '\0');


    return 0;
}
