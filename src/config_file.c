
/*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>

#include "config_file.h"


static FILE *open_config_file();
static void parse_config_file_data(FILE *file, struct config_file_data *data);


/*
    Add data from CONFIG_FILE to config_file_data
    ---------------
    - caller initializes config_file_data struct
*/
int get_config_file_data(struct config_file_data *data)
{
    if (data == NULL) {
        printf("config_file_data pointer is NULL\n");
        return 1;
    }

    FILE *config_file = open_config_file();

    if (config_file) {
        parse_config_file_data(config_file, data);
        fclose(config_file);
        return 0;
    } else {
        printf("Unable to open config file\n");
        return 1;
    }
}


/*
    Open CONFIG_FILE
    ---------------------
    - looks for .gdb-tuiffic in current and then HOME directory- example configuration in README.md
*/
static FILE *open_config_file()
{
    FILE *file;
    char cwd[100];
    char file_buffer[500];
    int width = 0;
    int curr_width = 0;
    int height = 0;

    // open config file
    char path[150];

    // check current directory
    getcwd(cwd, sizeof(cwd));
    snprintf(path, sizeof(path), "%s/%s", cwd, CONFIG_FILE);
    file = fopen(path, "r");
    if (file == NULL)
    {

        // check in HOME directory
        char *home = getenv("HOME");
        snprintf(path, sizeof(path), "%s/%s", home, CONFIG_FILE);
        file = fopen(path, "r");
    }

    // no config file
    if (file == NULL)
        return NULL;
        
    return file;
}

/*
    Parse config file, return config_file struct
*/
static void parse_config_file_data(
        FILE *file, 
        struct config_file_data *data)
{
    char ch;
    bool config_found = false;

    // config_file_data row indexes
    int i_cat = 0;
    int i_lab = 0;
    int i_val = 0;

    printf("%c\n", data->categories[0][0]);

    // read file
    while ((ch = fgetc(file)) != EOF) {

        int i;

        // get [<category>:<label>]
        if (ch == '[') {
            config_found = true;

            // get category
            i = 0;
            while ( (ch = fgetc(file)) != ':'  &&
                    i < MAX_CONFIG_CAT_LEN - 1  &&
                    i_cat < MAX_CONFIG_ENTRIES ) {

                data->categories[i_cat][i++] = ch;
            }
            data->categories[i_cat++][i] = '\0'; 

            // get label
            i = 0;
            while ( (ch = fgetc(file)) != ']' &&
                    i < MAX_CONFIG_CAT_LEN - 1  &&
                    i_cat < MAX_CONFIG_ENTRIES ) {

                data->categ_labels[i_lab][i++] = ch;
            }
            data->categ_labels[i_lab++][i] = '\0';
        }     

        // get [<category>:<label>] value
        if (config_found == true) {

            // add layout
            if (strcmp(data->categories[i_cat-1], "layout") == 0) {

                i = 0;
                bool is_val = false;
                while ( (ch = fgetc(file)) != '['  &&  ch != EOF ) {

                    if (isalpha(ch)) {
                        data->categ_values[i_val][i++] = ch;
                        is_val = true;
                    }

                    else if ( ch == '\n' && is_val == true && data->categ_values[i_val][i-1] != 'n') {
                        data->categ_values[i_val][i++] = 'n';
                    } 
                }
                data->categ_values[i_val][i] = '\0';

                i_val += 1;
                ungetc(ch, file);
                config_found = false;
            }
        }
    }
}

