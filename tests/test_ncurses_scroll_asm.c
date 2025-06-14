#include <stdlib.h>

#include "test_utilities.h"
#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/display_lines.h"
#include "../src/utilities.h"

#define DATA_PATH  "tests/mock.asm_out"
#define PLUG_IDX   Asm


int main (void)
{
    int ch;

    state_t *state = allocate_test_plugin(PLUG_IDX);
    window_t *win = state->plugins[PLUG_IDX]->win;
    win->buff_data->buff = create_buff_from_file (DATA_PATH);

    init_nc();
    create_nc_window (win);

    display_lines (ROW_DATA, PLUG_IDX, state);
    while ((ch = getch()) != 'q') {
        display_lines (ch, PLUG_IDX, state);
    }

    deinit_nc();

    return 0;
}

