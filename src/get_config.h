
/*
   Open, parse CONFIG_FILE
*/

#ifndef GET_CONFIG_H
#define GET_CONFIG_H

/*
    Get data from external configuration file
    ------
    Searches:
        PWD/CONFIG_FILE
        HOME/CONFIG_FILE

    Layouts

        Window type symbols

            status              t
            commands            c
            watches             w
            breakpoints         b
            assembly            a
            registers           r
            source code         s
            local variables     l

            newline symbol      n

        Example CONFIG_FILE layouts

            [layout:srcHoriz1]
            cs
            ws
            bs
            rs

            [ layout : src_ass ]
            s s a a

            s s a a

            c c b r

    More to come...
*/


#define CONFIG_FILE             ".gdb-tuiffic"
#define MAX_CONFIG_ENTRIES      20
#define MAX_CONFIG_CATEG_LEN    20
#define MAX_CONFIG_LABEL_LEN    MAX_CONFIG_CATEG_LEN
#define MAX_CONFIG_VALUE_LEN    100


/*
    Data structure to hold CONFIG_FILE data
*/
struct config_file_data {
    int     num_configs;
    char    categories      [MAX_CONFIG_ENTRIES][MAX_CONFIG_CATEG_LEN];
    char    categ_labels    [MAX_CONFIG_ENTRIES][MAX_CONFIG_LABEL_LEN];
    char    categ_values    [MAX_CONFIG_ENTRIES][MAX_CONFIG_VALUE_LEN];
};


/*
    Add data from CONFIG_FILE to config_file_data struct
    ----------

    Layout parsing example:

        CONFIG_FILE -->

            [layout:my_layout_1]
            cwbr
            aacc
            aacc

        get_config_file_data (data) -->

            num_configs       ==  1
            categories   [i]  ==  "layout"
            categ_labels [i]  ==  "my_layout_1"
            categ_values [i]  ==  "cwbrnaaccnaaccn"
*/
void get_config_file_data (struct config_file_data *data);


#endif
