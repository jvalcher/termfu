#include "../src/data.h"
#include "../src/start_debugger.h"
#include "../src/parse_cli_arguments.h"
#include "../src/get_popup_window_input/popup_breakpoints.h"



int
main (int argc, char *argv[])
{
    state_t *state = (state_t*) malloc (sizeof (state_t));;
    debugger_t *debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->debugger = debugger;
    breakpoint_t *curr_break;
    int row = 1;

    state->debugger->cli_buffer[0] = '\0';
    state->debugger->data_buffer[0] = '\0';
    state->debugger->program_buffer[0] = '\0';
    state->debugger->async_buffer[0] = '\0';

    initscr();

    parse_cli_arguments (argc, argv, debugger);
    start_debugger (state);

    // create breakpoints
    insert_breakpoint (state);
    insert_breakpoint (state);
    insert_breakpoint (state);

    /*
    mvwprintw (stdscr, 1, 5, "cli: %s\n", state->debugger->cli_buffer);
    mvwprintw (stdscr, 9, 5, "data: %s\n", state->debugger->data_buffer);
    mvwprintw (stdscr, 15, 5, "prog: %s\n", state->debugger->program_buffer);
    mvwprintw (stdscr, 21, 5, "async: %s\n", state->debugger->async_buffer);
    */

    // print breakpoints
    curr_break = state->breakpoints;
    while (curr_break != NULL) {
        mvwprintw (stdscr, row, 3, "%s", curr_break->path_line);
        curr_break = curr_break->next;
        row += 2;
    }
    refresh();
    getch();

    // delete breakpoint
    delete_breakpoint (state);

    // print breakpoints
    curr_break = state->breakpoints;
    while (curr_break != NULL) {
        mvwprintw (stdscr, row, 3, "%s", curr_break->path_line);
        curr_break = curr_break->next;
        row += 2;
    }
    refresh();
    getch();

    endwin ();

    return 0;
}
