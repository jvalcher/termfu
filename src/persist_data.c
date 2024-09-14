#include <string.h>
#include <ctype.h>

#include "persist_data.h"
#include "data.h"
#include "update_window_data/_update_window_data.h"
#include "plugins.h"
#include "insert_output_marker.h"
#include "parse_debugger_output.h"
#include "utilities.h"

static watchpoint_t *create_watchpoint (state_t *state);



/*
    Get persisted data from PERSIST_FILE

    Format:
    ------
    >w
    <index>:<variable>
    >b
    <file>:<line>
*/
void
get_persisted_data (state_t *state)
{
    FILE *fp;
    int ch, i;
    watchpoint_t *watch;
    char  break_buff [BREAK_LEN],
         *cmd_base_gdb = "-break-insert ",
         *cmd_base_pdb = "break ",
         *cmd_base,
         *cmd;

    switch (state->debugger->index) {
        case DEBUGGER_GDB: cmd_base = cmd_base_gdb; break;
        case DEBUGGER_PDB: cmd_base = cmd_base_pdb; break;
    }

    if ((fp = fopen (PERSIST_FILE, "r")) != NULL) {

        while ((ch = fgetc (fp)) != EOF) {

            if (ch == '>') {

                ch = fgetc (fp);

                // watchpoints
                if (ch == 'w') {
                    while ((ch = fgetc (fp)) != '>' && ch != EOF) {

                        if (isalpha (ch)) {

                            ungetc (ch, fp);

                            watch = create_watchpoint (state);

                            // variable
                            i = 0;
                            while ((ch = fgetc (fp)) != '\n' && ch != EOF) {
                                watch->var[i++] = (char) ch;
                            }
                            watch->var[i] = '\0';
                        }
                    }
                    ungetc (ch, fp);
                }

                // breakpoints
                else if (ch == 'b') {
                    while ((ch = fgetc (fp)) != EOF && ch != '[') {

                        if (isalnum (ch) || ch == '/') {

                            // get breakpoint <path>:<line>
                            i = 0;
                            ungetc (ch, fp);
                            while ((ch = fgetc (fp)) != '\n' && ch != EOF) {
                                break_buff [i++] = (char) ch;
                            }
                            break_buff [i] = '\0';

                            // insert breakpoint
                            cmd = concatenate_strings (3, cmd_base, break_buff, "\n");    

                            insert_output_start_marker (state);
                            send_command (state, cmd);
                            insert_output_end_marker (state);
                            parse_debugger_output (state);

                            free (cmd);
                        }
                    }
                }

            }
        }

        fclose (fp);
    }
    
    update_windows (state, 2, Wat, Brk);
}



void
persist_data (state_t *state)
{
    FILE *fp;
    breakpoint_t *curr_break;
    watchpoint_t *curr_watch;

    fp = fopen (PERSIST_FILE, "w");
    if (fp == NULL) {
        pfeme ("Failed to open data persistence file\n");
    }

    // watchpoints
    curr_watch = state->watchpoints;
    if (curr_watch != NULL) {

        fprintf (fp, ">w\n");

        do {
            fprintf (fp, "%s\n", curr_watch->var);
            curr_watch = curr_watch->next;
        } while (curr_watch != NULL);
    }

    // breakpoints
    curr_break = state->breakpoints;
    if (curr_break != NULL) {

        fprintf (fp, ">b\n");

        do {
            fprintf (fp, "%s\n", curr_break->path_line);
            curr_break = curr_break->next;
        } while (curr_break != NULL);
    }

    fclose (fp);
}



static watchpoint_t*
create_watchpoint (state_t *state)
{
    int index;
    watchpoint_t *watch = NULL;

    // first watchpoint
    if (state->watchpoints == NULL) {
        state->watchpoints = (watchpoint_t*) malloc (sizeof (watchpoint_t));
        watch = state->watchpoints;
        if (watch == NULL) {
            pfeme ("Unable to allocate watchpoint\n");
        }
        watch->var[0] = '\0';
        watch->index = 1;
        watch->next = NULL;
    } 

    // new watchpoint
    else {
        watch = state->watchpoints;
        index = watch->index;
        while (watch->next != NULL) {
            watch = watch->next; 
            index = watch->index;
        }
        watch->next = (watchpoint_t*) malloc (sizeof (watchpoint_t));
        watch = watch->next;
        if (watch == NULL) {
            pfeme ("Unable to allocate watchpoint\n");
        }
        watch->index = index + 1;
        watch->value[0] = '\0';
        watch->next = NULL;
    }

    return watch;
}

