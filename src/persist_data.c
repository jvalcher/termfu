#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "persist_data.h"
#include "data.h"
#include "update_window_data/_update_window_data.h"
#include "plugins.h"
#include "utilities.h"

static watchpoint_t *create_watchpoint (state_t *state);



/*
    Format
    ------
    >w  watchpoints
    >b  breakpoints

    >w
    <index>:<variable>
    >b
    <file>:<line>
*/
int
get_persisted_data (state_t *state)
{
    FILE         *fp;
    int           ch, i,
                  ret;
    watchpoint_t *watch;
    char  break_buff [BREAK_PATH_LEN],
         *cmd_base_gdb = "-break-insert ",
         *cmd_base_pdb = "break ",
         *cmd_base = NULL,
         *cmd;

    switch (state->debugger->index) {
        case DEBUGGER_GDB:
            cmd_base = cmd_base_gdb;
            break;
        case DEBUGGER_PDB:
            cmd_base = cmd_base_pdb;
            break;
    }

    // open file
    if (state->data_path[0] != '\0') {
        fp = fopen (state->data_path, "r");
    } else {
        fp = fopen (PERSIST_FILE, "r");
    }

    if (fp != NULL) {
        while ((ch = fgetc (fp)) != EOF) {

            if (ch == '>') {

                ch = fgetc (fp);

                // watchpoints
                if (ch == 'w') {
                    while ((ch = fgetc (fp)) != '>' && ch != EOF) {

                        if ((watch = create_watchpoint (state)) == NULL) {
                            pfemr ("Failed to create watchpoint");
                        }

                        // variable
                        i = 0;
                        while ((ch = fgetc (fp)) != '\n' && ch != EOF) {
                            watch->var[i++] = (char) ch;
                        }
                        watch->var[i] = '\0';
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

                            ret = send_command_mp (state, cmd);
                            if (ret == FAIL) {
                                pfemr (ERR_DBG_CMD);
                            }

                            free (cmd);
                        }
                    }
                }

            }
        }

        fclose (fp);
    }

    ret = update_windows (state, 4, Wat, Brk, Src, Asm);
    if (ret == FAIL) {
        pfemr (ERR_UPDATE_WINS);
    }

    return A_OK;
}



int
persist_data (state_t *state)
{
    FILE *fp;
    breakpoint_t *curr_break;
    watchpoint_t *curr_watch;

    // open file
    if (state->data_path[0] != '\0') {
        if ((fp = fopen (state->data_path, "w")) == NULL) {
            pfem ("fopen: %s", strerror (errno));
            pemr ("Failed to open data persistence file \"%s\"\n", state->data_path);
        }
    } else {
        if ((fp = fopen (PERSIST_FILE, "w")) == NULL) {
            pfem ("fopen: %s", strerror (errno));
            pemr ("Failed to open default data persistence file \"%s\"\n", PERSIST_FILE);
        }
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
            fprintf (fp, "%s:%s\n", curr_break->path, curr_break->line);
            curr_break = curr_break->next;
        } while (curr_break != NULL);
    }

    fclose (fp);

    return A_OK;
}



static watchpoint_t*
create_watchpoint (state_t *state)
{
    int index;
    watchpoint_t *watch = NULL;

    // first watchpoint
    if (state->watchpoints == NULL) {
        if ((state->watchpoints = (watchpoint_t*) malloc (sizeof (watchpoint_t))) == NULL) {
            pfem ("malloc error: %s", strerror (errno));
            pem  ("Failed to allocate state->watchpoints (first)");
            return NULL;
        }
        watch = state->watchpoints;
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
        if ((watch->next = (watchpoint_t*) malloc (sizeof (watchpoint_t))) == NULL) {
            pfem ("malloc error: %s", strerror (errno));
            pem  ("Failed to allocate state->watchpoints");
            return NULL;
        }
        watch = watch->next;
        watch->index = index + 1;
        watch->value[0] = '\0';
        watch->next = NULL;
    }

    return watch;
}

