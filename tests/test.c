
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#include <unistd.h>


int main (void)
{
    char *str = "Hello, world!";

    initscr ();

    mvwprintw (stdscr, 1, 1, "%.*s\n", 3, str);
    refresh ();

    getch (); 
    endwin ();

    return 0;
}
