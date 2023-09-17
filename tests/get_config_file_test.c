
/*
   Test src/config_file.c
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../src/get_config_file.h"

int main (void) 
{
    // get config data
    struct config_file_data data = {0};
    get_config_file_data (&data);

    // print info
    for (int i = 0; i < MAX_CONFIG_ENTRIES; i++) {
        if (data.categories[i][0] != 0) {
            printf ("[%s:%s]\n%s\n", 
                    data.categories[i],
                    data.categ_labels[i],
                    data.categ_values[i] );
        }
    }
    printf("\nNumber of configs: %d\n", data.num_configs);

    return 0;
}

