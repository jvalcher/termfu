
/*
    Apply configuration file
*/

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#include "apply_config.h"
#include "apply_config_layout.h"
#include "get_config.h"
#include "utilities.h"


/*
    Parse config_file_data struct (get_config.h)
    Pass resulting data into configuration structs (apply_config_*.h)
*/
void apply_config (struct layouts *layouts)
{
    // calculate current terminal pane dimensions
    int scr_cols = getmaxx (stdscr);
    int scr_rows = getmaxy (stdscr) - 2;   // minus program title bar

    // get config file data
    struct config_file_data data = {0};
    get_config_file_data (&data);

    // create configuration structs for
        // layouts
    layouts = (struct layouts*) malloc (sizeof (struct layouts));
    if (layouts == NULL) {
        endwin ();
        pfem ("layouts struct is null");
        exit (EXIT_FAILURE);
    }
    layouts->num_layouts = 0;

    // loop through configurations
    for (int i = 0; i < data.num_configs; i++) {

        // if layout
        if (strcmp (data.categories[i], "layout") == 0) {
            layouts->num_layouts += 1;
            apply_config_layout (i, scr_rows, scr_cols, &data, layouts);
        }                        
    }
}

