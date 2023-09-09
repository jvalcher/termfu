
/*
   Test src/config_file.c
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../src/config_file.h"

int main (void) 
{
    /*
        struct config_file_data {
            char categories     [MAX_CONFIG_ENTRIES][MAX_CONFIG_LEN];
            char categ_labels   [MAX_CONFIG_ENTRIES][MAX_CONFIG_LEN];
            char categ_values   [MAX_CONFIG_ENTRIES][MAX_CONFIG_LEN];
        };
    */
    struct config_file_data *data = calloc(1, sizeof(struct config_file_data));
    memset(data, 0, sizeof(struct config_file_data));

    if (data) {
        if (get_config_file_data(data) == 0) {

            for (int i = 0; i < MAX_CONFIG_ENTRIES; i++) {
                if (data->categories[i][0] != 0) {
                    printf("[%s:%s]\n%s\n", 
                            data->categories[i],
                            data->categ_labels[i],
                            data->categ_values[i]);
                }
            }
        } 
        else
            puts("No config file found");

        free(data);
    } 
    else
        puts("TEST: Unable to get config file data");


    return 0;
}

