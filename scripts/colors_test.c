
/*
    Display terminal color capabilities
    -----------
    Used by /display_colors script
*/

#include <ncurses.h>

#define PRINT_COLOR(fg, bg, macro) \
    init_pair((fg), (fg), (bg)); \
    attron(COLOR_PAIR((fg))); \
    printw("%s\t %d  \t %d  \n", (macro), (fg), (bg)); \
    refresh(); \
    attrset(A_NORMAL);

int main(void)
{
    initscr();

    if (has_colors()) {

        start_color();

        printw("\nTerminal colors enabled\n\n");

        printw("\n---------------\n");
        printw("\nTERMINAL COLORS ENABLED\n\n");
        printw("\n---------------\n");

        printw("Available\n");
        printw("---------\n");
        printw("%d colors\n", COLORS);
        printw("%d color pairs\n", COLOR_PAIRS);
        refresh();

        printw("\n\nBackground");
        printw("\n------------\n");

        printw("Reversed... ");
        refresh();
        init_pair(10, 0, 7);
        bkgd(COLOR_PAIR(10));
        refresh();
        napms(1000);
        init_pair(11, 7, 0);
        bkgd(COLOR_PAIR(11));
        printw("back to normal\n");
        refresh();

        printw("\".\" dot background... skipped\n\n\n");
        /*  "." dots background
        printw("\".\" background... ");
        bkgd('.');
        refresh();
        napms(1000);
        bkgd(' ' | COLOR_PAIR(11));
        printw("\tback to normal\n\n\n");
        refresh();
        */

        printw("Notify\n");
        printw("------------\n");
        printw("<BEEP>\n");
        refresh();
        napms(1000);
        beep();
        printw("<FLASH>\n\n\n");
        refresh();
        napms(1000);
        flash();


        /* print colors */
        printw("fg MACRO \t fg \t bg \n");
        printw("-------- \t----\t----\n");
        refresh();
        PRINT_COLOR(2, 0, "COLOR_GREEN");
        PRINT_COLOR(3, 0, "COLOR_YELLOW");
        PRINT_COLOR(4, 0, "COLOR_BLUE");
        PRINT_COLOR(5, 0, "COLOR_MAGENTA");
        PRINT_COLOR(6, 0, "COLOR_CYAN");
        PRINT_COLOR(7, 0, "COLOR_WHITE");

        printw("\n\nReversed colors");
        printw("\n-----------------\n");
        refresh();
        attron(A_REVERSE);
        PRINT_COLOR(2, 0, "COLOR_GREEN");

        printw("\n\nMixed colors");
        printw("\n------------\n");
        refresh();
        if (can_change_color()) {
            // create RGB color 34  (pink: 100, 75, 75)
            init_color(34, 1000, 750, 750);
            PRINT_COLOR(34, 0, "pink: 100, 75, 75");
        } else {
            printw("can_change_color() returned false: \nTerminal unable to mix colors\n");
        }


    // terminal colors not enabled
    } else {
        printw("Terminal unable to display colors\n");
        refresh();
    }

    printw("\n\nPress any key to quit\n");
    refresh();
    getch();
    endwin();

    return 0;
}
