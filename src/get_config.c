#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <ncurses.h>

#include "utilities.h"
#include "get_config.h"

static FILE *open_config_file ();
static void  parse_config_file_data (FILE* file, 
                                     struct config_file_data* data);


/*
    Add data from CONFIG_FILE to config_file_data struct
*/
void get_config_file_data (struct config_file_data *data)
{
    // check data
    if (data == NULL) {
        endwin();
        pfem ("config_file_data struct pointer is NULL");
        exit (EXIT_FAILURE);
    }

    // open config file
    FILE *config_file = open_config_file ();

    // parse it
    parse_config_file_data (config_file, data);
}


/*
    Open CONFIG_FILE
    ----------
    Looks for CONFIG_FILE in PWD and HOME directories
*/
static FILE *open_config_file ()
{
    FILE *file = NULL;
    char cwd[100];
    char cwd_path[130];
    char home_path[130];

    // check current working directory
    if (getcwd (cwd, sizeof (cwd)) != NULL) {
        snprintf (cwd_path, sizeof (cwd_path), "%s/%s", cwd, CONFIG_FILE);
        file = fopen (cwd_path, "r");
    }

    // check in user's home directory
    if (file == NULL) {
        char *home = getenv ("HOME");
        snprintf (home_path, sizeof (home_path), "%s/%s", home, CONFIG_FILE);
        file = fopen (home_path, "r");
    }

    // if no config file found, exit
    if (file == NULL) {

        endwin ();

        pfem ("Unable to find config file in home or current working directory");
        pfemo (cwd_path);
        pfemo (home_path);

        exit (EXIT_FAILURE);
    }
        
    return file;
}


/*
    Parse configuration file
    ------------------------
    - Parse config file buffer <file>
    - Add settings to passed
        config_file.h --> config_file_data
*/
static void parse_config_file_data (
        FILE *file, 
        struct config_file_data *data )
{
    if (file == NULL) {
        endwin();
        pfem ("FILE pointer is NULL");
        exit (EXIT_FAILURE);
    }

    char ch;
    bool config_found = false;
    data->num_configs = 0;

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
                    i < MAX_CONFIG_CATEG_LEN - 1  &&
                    i_cat < MAX_CONFIG_ENTRIES ) {

                data->categories[i_cat][i++] = ch;
            }
            data->categories[i_cat++][i] = '\0'; 

            // get label
            i = 0;
            while ( (ch = fgetc(file)) != ']' &&
                    i < MAX_CONFIG_CATEG_LEN - 1  &&
                    i_cat < MAX_CONFIG_ENTRIES ) {

                data->categ_labels[i_lab][i++] = ch;
            }
            data->categ_labels[i_lab++][i] = '\0';
            data->num_configs += 1;
        }     

        // get config value
        if (config_found == true) {

            // add layout
            if (strcmp(data->categories[i_cat-1], "layout") == 0) {

                // create data->categ_values[i_val] string
                i = 0;
                bool is_val = false;
                while ( (ch = fgetc(file)) != '['  &&  ch != EOF ) {

                    // add letter
                    if (isalpha(ch)) {
                        data->categ_values[i_val][i++] = ch;
                        is_val = true;
                    }

                    // add 'n' for newline
                    else if (   ch == '\n' && 
                                is_val == true && 
                                data->categ_values[i_val][i-1] != 'n' // no duplicates
                            )   {
                        data->categ_values[i_val][i++] = 'n';
                    } 
                }
                data->categ_values[i_val][i] = '\0';
                i_val += 1;
                ungetc(ch, file);       // unget '['
                config_found = false;
            }
        }
    }

    fclose (file);
}
