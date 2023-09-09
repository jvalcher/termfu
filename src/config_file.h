/*
    Import settings from CONFIG_FILE

    Layouts
    -------
        Windows:
            - status                t
            - commands              c
            - watches               w
            - breakpoints           b
            - assembly              a
            - registers             r
            - source code           s

        * n reserved for newline symbol

        Examples usage:
            
            [layout:srcHoriz1]
            cs
            ws
            bs
            rs

            [ layout : src_ass ]
            s w b r

            s s a a

            c c a a
*/


#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H


#define CONFIG_FILE         ".gdb-tuiffic"
#define MAX_CONFIG_ENTRIES  20
#define MAX_CONFIG_CAT_LEN  20
#define MAX_CONFIG_VAL_LEN  100


/*
    CONFIG_FILE data
*/
struct config_file_data {
    char categories     [MAX_CONFIG_ENTRIES][MAX_CONFIG_CAT_LEN];
    char categ_labels   [MAX_CONFIG_ENTRIES][MAX_CONFIG_CAT_LEN];
    char categ_values   [MAX_CONFIG_ENTRIES][MAX_CONFIG_VAL_LEN];
};


/*
    Add data from CONFIG_FILE
    ---------------
    - caller creates,frees config_file_data struct
*/
int get_config_file_data (struct config_file_data *data);


#endif
