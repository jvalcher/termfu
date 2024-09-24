
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

#include "pulse_header_title_color.h"
#include "data.h"
#include "utilities.h"

#define MIN_PULSE_LEN  .06


struct timeval start, end;



int
pulse_header_title_color (int plugin_index,
                          state_t *state,
                          int pulse_state)
{
    size_t  i;
    int     x, y,
            title_color,
            keych_color;
    double  diff;
    bool    key_color_toggle,
            string_exists;
    WINDOW *header;
    char   *title;

    // set colors, handle pulse length
    switch (pulse_state) {
        case ON:
            if (gettimeofday (&start, NULL) == -1) {
                pfemr ("Failed to get start time of day (ON): %s", strerror (errno));
            }
            title_color = FOCUS_HEADER_TITLE_COLOR;
            keych_color = FOCUS_TITLE_KEY_COLOR;
            break;
        case OFF:
            if (gettimeofday(&end, NULL) == -1) {
                pfemr ("Failed to get end time of day (OFF): %s", strerror (errno));
            }
            diff = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
            if (diff < MIN_PULSE_LEN) {
                usleep (MIN_PULSE_LEN * 2000000);
            }
            title_color = HEADER_TITLE_COLOR;
            keych_color = TITLE_KEY_COLOR;
    }

    header = state->header;
    title  = state->plugins[plugin_index]->title;
    
    // get title string location
    if ((string_exists = find_window_string (header, title, &y, &x)) == false) {
        pfemr ("Unable to find header title \"%s\"", title);
    }

    // change title string color
    if (string_exists) {

        key_color_toggle = false;
        if (set_nc_attribute (header, title_color) == RET_FAIL) {
            pfemr ("Failed to set ncurses attribute");
        }
        for (i = 0; i < strlen (title) + 1; i++) {
            mvwprintw (header, y, x + i, "%c", title [i]);
            if (key_color_toggle) {
                if (set_nc_attribute (header, title_color) == RET_FAIL) {
                    pfemr ("Failed to set ncurses attribute");
                }
                key_color_toggle = false;
            }
            if (title [i] == '(') {
                if (set_nc_attribute (header, keych_color) == RET_FAIL) {
                    pfemr ("Failed to set ncurses attribute");
                }
                key_color_toggle = true;
            }
        }
        wrefresh (header);
    }

    return RET_OK;
}
