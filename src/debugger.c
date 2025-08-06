#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <termio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>

#include "debugger.h"
#include "error.h"
#include "utils.h"

#define DBG_PIPE_READ      0
#define DBG_PIPE_WRITE     1
#define DBG_TITLE_LEN      4
#define DBG_READ_BUF_SIZE  3072
#define DBG_OUT_BUF_SIZE   12288
#define DBG_CMD_SIZE       512

const char *end_marker = ">END";

#define DBG_GDB_BIN_INDEX    2
#define DBG_PDB_SCRPT_INDEX  3
char *commands[][5] = {             // Indexes match *_DEBUGGER enums in header
    { "gdb", "--interpreter=mi", NULL, NULL, NULL },
    { "python", "-m", "pdb", NULL, NULL }
};
const char *cmd_titles[] = {        // Indexes match *_DEBUGGER enums in header
    "gdb",
    "pdb"
};

typedef struct debugger {

    bool initialized;   // Initialized flag
    int index;          // Debugger index
    const char *title;  // e.g. "gdb"
    char **command;     // e.g. { "python", "-m", "pdb", "app.py", NULL }
    char last_cmd[DBG_CMD_SIZE];  // Last command run

    // Subprocess
    int pid;            // Subprocess pid
    bool running;       // Running flag
    int stdin_pipe;     // Input pipe
    int stdout_pipe;    // Ouput pipe

    // Output read buffer
    bool reading;
    char read_buf[DBG_READ_BUF_SIZE];

    // Parsed output buffers
    buffer_t *data_buf;
    buffer_t *cli_buf;
    buffer_t *async_buf;
    buffer_t *prog_buf;

} debugger_t;

char** create_command_impl(int max_strs, ...)
{
    char **command_arr;
    char *str;

    va_list strs;

    // Allocate command array
    int str_count = 0;
    va_start(strs, max_strs);
    for (str = va_arg(strs, char*); str != NULL; str = va_arg(strs, char*)) {
        ++str_count;
        if (str_count >= max_strs)
            pfemn("Max strings exceeded");
    }
    va_end(strs);
    if (!(command_arr = malloc((str_count + 1) * sizeof(char*))))
        pfemn("Failed to allocate command array");
    
    // Allocate strings
    va_start(strs, max_strs);
    for (int i = 0; i < str_count; i++) {
        str = va_arg(strs, char*);
        size_t len = strlen(str) + 1;
        if (!(command_arr[i] = malloc(len)))
            pfemn("Failled to allocate command string");
        strncpy(command_arr[i], str, len); 
    }
    va_end(strs);

    command_arr[str_count] = NULL;

    return command_arr;
}

void destroy_command(char** command)
{
    for (int i = 0; i < DBG_MAX_STRS; i++) {
        if (command[i] == NULL)
            break;
        free(command[i]);
    }
    free(command);
}

debugger_t* init_debugger(int dbg_index, char** command)
{
    int num_debuggers = sizeof(commands) / sizeof(commands[0]);
    if (dbg_index < 0 || dbg_index >= num_debuggers)
        pfemn("Debugger index not recognized (%d)", dbg_index);



    debugger_t *d;
    if (!(d = malloc(sizeof(debugger_t)))) {
        pfemn_errno("Failed to allocate debugger_t struct");
    }

    d->index = dbg_index;
    d->last_cmd[0] = '\0';

    switch(dbg_index) {
        case GDB_DEBUGGER:
            d->title = cmd_titles[GDB_DEBUGGER];
            break;
        case PDB_DEBUGGER:
            d->title = cmd_titles[PDB_DEBUGGER];
            break;
    }

    d->command = command;
    d->read_buf[0] = '\0';

    if (!(d->cli_buf = create_buffer(DBG_OUT_BUF_SIZE)))
        goto cli_err;
    if (!(d->data_buf = create_buffer(DBG_OUT_BUF_SIZE)))
        goto data_err;
    if (!(d->async_buf = create_buffer(DBG_OUT_BUF_SIZE)))
        goto async_err;
    if (!(d->prog_buf = create_buffer(DBG_OUT_BUF_SIZE)))
        goto prog_err;

    d->initialized = true;

    return d;

    char *buf_title;
prog_err:
    buf_title = "program";
    destroy_buffer(d->async_buf);
async_err:
    buf_title = "async";
    destroy_buffer(d->data_buf);
data_err:
    destroy_buffer(d->cli_buf);
    buf_title = "data";
cli_err:
    buf_title = "cli";
    free(d);
    pfemn("Failed to create %s buffer", buf_title);
}

static void start_debugger_proc(debugger_t *d)
{
    pid_t debug_pid;
    int debug_in_pipe[2],
        debug_out_pipe[2];

    if (pipe(debug_in_pipe) == -1 || pipe(debug_out_pipe) == -1)
        pfeme("Debugger pipe creation failed");

    debug_pid = fork();
    if (debug_pid == -1)
        pfeme("Debugger fork failed");

    if (debug_pid == 0) {

        dup2(debug_in_pipe[DBG_PIPE_READ], STDIN_FILENO);
        close(debug_in_pipe[DBG_PIPE_READ]);
        close(debug_in_pipe[DBG_PIPE_WRITE]);

        dup2(debug_out_pipe[DBG_PIPE_WRITE], STDOUT_FILENO);
        close(debug_out_pipe[DBG_PIPE_WRITE]);
        close(debug_out_pipe[DBG_PIPE_READ]);

        execvp(d->command[0], d->command);

        pfeme_errno("Failed to start debugger \"%s\"", d->title);
    }

    if (debug_pid > 0) {
    
        d->pid = debug_pid;

        d->stdin_pipe = debug_in_pipe[DBG_PIPE_WRITE];
        d->stdout_pipe = debug_out_pipe[DBG_PIPE_READ];
        close(debug_in_pipe[DBG_PIPE_READ]);
        close(debug_out_pipe[DBG_PIPE_WRITE]);
    }
}

/*
    GDB/MI first line character flags
    ------------
    ~ -> d->cli_buffer     - GDB CLI console window output
    ^ -> d->data_buffer    - Data values e.g. source file path, line number, breakpoints
    * -> d->async_buffer   - Async state change info e.g. started, stopped
      -> d->program_buffer - Debugged program's CLI output
*/
static void parse_debugger_output_gdb(debugger_t *d)
{
    bool is_cli_output = false;      // ~
    bool is_data_output = false;     // ^
    bool is_async_output = false;    // *
    bool is_prog_output = false;     // 
    bool is_newline = true;

    char *buf_ptr = d->read_buf;

    while (*buf_ptr != '\0') {

        // Set line's output type
        if (is_newline && *buf_ptr != '\n') {
            is_newline = false;
            if (*buf_ptr == '~') {
                is_cli_output = true;
                ++buf_ptr;
            } else if (*buf_ptr == '^') {
                is_data_output = true;
                ++buf_ptr;
            } else if (*buf_ptr == '*') {
                is_async_output = true;
                ++buf_ptr;
            } else if (isalnum(*buf_ptr)) {
                is_prog_output = true;
            }
        }

        // End of line
        else if (*buf_ptr == '\n') {
            is_newline = true;
            if (is_cli_output) {
                is_cli_output = false;
                concat_buf_ch(d->cli_buf, *buf_ptr++);
            } else if (is_prog_output) {
                is_prog_output = false;
                concat_buf_ch(d->prog_buf, *buf_ptr++);
            } else if (is_data_output) {
                is_data_output = false;
                concat_buf_ch(d->data_buf, *buf_ptr++);
            } else if (is_async_output) {
                is_async_output = false;
                concat_buf_ch(d->async_buf, *buf_ptr++);
            } else {
                ++buf_ptr;
            }
        }

        else if (is_cli_output) {

            //  \\\t, \\\n
            if (*buf_ptr == '\\' && isalpha(*(buf_ptr + 1))) {
                concat_buf_ch(d->cli_buf, *buf_ptr++);
                if (*(buf_ptr + 1) == 'n') {
                    concat_buf_ch(d->cli_buf, 'n');
                    buf_ptr += 2;
                } else if (*(buf_ptr + 1) == 't') {
                    concat_buf_ch(d->cli_buf, 't');
                    buf_ptr += 2;
                } 
            }

            //  \\\"  ->  \"
            else if (*buf_ptr == '\\' && *(buf_ptr + 1) == '\"' ) {
                buf_ptr += 1;
                concat_buf_ch(d->cli_buf, *buf_ptr++);
            }

            //  \"  ->  skip
            else if (*buf_ptr == '\"') {
                buf_ptr += 1;
            }

            //  \\\\  ->  skip
            else if (*buf_ptr == '\\' && *(buf_ptr + 1) == '\\' ) {
                buf_ptr += 1;
            }

            //  '>'
            else if (*buf_ptr == '>') {

                //  end of command output marker
                //
                //     ">END\n"
                //
                if (strstr(buf_ptr, end_marker)) {
                    d->reading = false;
                    break;
                }

                else {
                    concat_buf_ch(d->cli_buf, *buf_ptr++);
                }
            }

            else {
                concat_buf_ch(d->cli_buf, *buf_ptr++);
            }

        }

        else if (is_prog_output) {
            concat_buf_ch(d->prog_buf, *buf_ptr++);
        }

        else if (is_data_output) {
            concat_buf_ch(d->data_buf, *buf_ptr++);
        }

        else if (is_async_output) {
            concat_buf_ch(d->async_buf, *buf_ptr++);
        }

        else {
            ++buf_ptr;
        }
    }
}

/*
    Parse PDB output
    ---------
    Debugger output ->cli_buf
    Program output ->prog_buf
*/
static void parse_debugger_output_pdb(debugger_t *d)
{
    char *break_str    = "Breakpoint 1 at /",
         *restart_str  = "Restarting /",
         *return_str   = "--Return--",
         *call_str     = "--Call--",
         *where_s_str  = "  /",
         *where_c_str  = "  <",
         *finished_str = "The program finished and will be restarted";

    char *buf_ptr = d->read_buf;

    while (*buf_ptr != '\0') {

        // newline
        if (*(buf_ptr - 1) == '\n' || buf_ptr == d->read_buf) {

            // ->
            if ( *buf_ptr      == '-' &&
                *(buf_ptr + 1) == '>' &&
                *(buf_ptr + 2) == ' ')
            {
                do {
                    concat_buf_ch(d->cli_buf, *buf_ptr++);
                } while (*(buf_ptr - 1) != '\n');
            }

            // >
            else if (*buf_ptr == '>') {

                // > /path/...
                if (*(buf_ptr + 1) == ' ' &&
                    (*(buf_ptr + 2) == '/' || *(buf_ptr + 2) == '<'))
                do {
                    concat_buf_ch(d->cli_buf, *buf_ptr++);
                } while (*(buf_ptr - 1) != '\n' && *buf_ptr != '\0');

                // program output
                else {
                    do {
                        concat_buf_ch(d->prog_buf, *buf_ptr++);
                    } while (*(buf_ptr - 1) != '\n' && *buf_ptr != '\0');
                }
            }

            // *** msg
            else if ( *buf_ptr      == '*' &&
                     *(buf_ptr + 1) == '*' &&
                     *(buf_ptr + 2) == '*')
            {
                do {
                    concat_buf_ch(d->cli_buf, *buf_ptr++);
                } while (*(buf_ptr - 1) != '\n' && *buf_ptr != '\0');
            }

            // skip prompt
            else if ( *buf_ptr      == '('  &&
                     *(buf_ptr + 1) == 'P'  &&
                     *(buf_ptr + 2) == 'd'  &&
                     *(buf_ptr + 3) == 'b'  &&
                     *(buf_ptr + 4) == ')')
                {

                buf_ptr  += 6;

                if (*buf_ptr == '\'') {
                    if (strstr(buf_ptr, end_marker)) {
                        d->reading = false;
                        break;
                    }
                }
            }

            // read() sometimes puts this as first line instead of following (pdb)
            else if (*buf_ptr == '\'') {

                // end output
                if (strstr(buf_ptr, end_marker)) {
                    d->reading = false;
                    break;
                }
            }

            // misc cli strings
            else if (strncmp(buf_ptr, break_str, strlen (break_str)) == 0 || 
                     strncmp(buf_ptr, return_str, strlen (return_str)) == 0 || 
                     strncmp(buf_ptr, finished_str, strlen (finished_str)) == 0 || 
                     strncmp(buf_ptr, where_s_str, strlen (where_s_str)) == 0 || 
                     strncmp(buf_ptr, where_c_str, strlen (where_c_str)) == 0 || 
                     strncmp(buf_ptr, call_str, strlen (call_str)) == 0 || 
                     strncmp(buf_ptr, restart_str, strlen (restart_str)) == 0) {
                do {
                    concat_buf_ch(d->cli_buf, *buf_ptr++);
                } while (*(buf_ptr - 1) != '\n' && *buf_ptr != '\0');
            }

            // program output
            else {
                do {
                    concat_buf_ch(d->prog_buf, *buf_ptr++);
                } while (*(buf_ptr - 1) != '\n' && *buf_ptr != '\0');
            }
        }

        // sometimes "(pdb)" inline
        else if ( *buf_ptr      == '('  &&
                 *(buf_ptr + 1) == 'P'  &&
                 *(buf_ptr + 2) == 'd'  &&
                 *(buf_ptr + 3) == 'b'  &&
                 *(buf_ptr + 4) == ')') {
            buf_ptr    += 6;
            *buf_ptr++ = '\n';
        }

        else {
            concat_buf_ch(d->cli_buf, *buf_ptr++);
        }
    }
}

static int parse_debugger_output(debugger_t *d)
{
    ssize_t bytes_read = 0;

    d->reading = true;
    d->read_buf[0] = '\0';

    clear_buffer(d->data_buf);
    clear_buffer(d->cli_buf);
    clear_buffer(d->async_buf);
    clear_buffer(d->prog_buf);

    while (d->reading) {

        // Read
        bytes_read = read(d->stdout_pipe, 
                          d->read_buf,
                          DBG_READ_BUF_SIZE - 1);
        if (bytes_read == -1)
            pfemr_errno("Failed to read debugger stdout");
        d->read_buf[bytes_read] = '\0';

        // Wait for debugger subprocess to start
        if (!d->running) {
            switch(d->index) {
                case GDB_DEBUGGER:
                    if (strstr(d->read_buf, "(gdb)")) {
                        d->running = true;
                        d->reading = false;
                    }
                    break;
                case PDB_DEBUGGER:
                    if (strstr(d->read_buf, "(Pdb)")) {
                        d->running = true;
                        d->reading = false;
                    }
                    break;
            }
            continue;
        } 

        // Parse output
        switch (d->index) {
            case GDB_DEBUGGER:
                parse_debugger_output_gdb(d);
                break;
            case PDB_DEBUGGER:
                parse_debugger_output_pdb(d);
                break;
        }

    }

    return A_OK;
}

/*
    Debugger output buffer getters
*/
char* cli_buffer(debugger_t *d)
{
    return buffer(d->cli_buf);
}
char* data_buffer(debugger_t *d)
{
    return buffer(d->data_buf);
}
char* async_buffer(debugger_t *d)
{
    return buffer(d->async_buf);
}
char* program_buffer(debugger_t *d)
{
    return buffer(d->prog_buf);
}

/*
    Send string(s) to debugger without end marker
*/
int send_command_impl(debugger_t *d, int max_strs, ...)
{
    int str_count = 0;
    char *str;
    va_list strs;

    if (!d->running)
        pfemr ("Debugger \"%s\" not running, unable to send command", d->title);

    va_start (strs, max_strs);
    for (str = va_arg(strs, char*); str != NULL; str = va_arg(strs, char*)) {

        if (++str_count > max_strs)
            break;

        if (write (d->stdin_pipe, str, strlen(str)) == -1)
            pfemr_errno ("Command write error on \"%s\"", str);
    }
    va_end (strs);

    return A_OK;
}

/*
    Insert output end marker
    -------
    - Ensures all debugger output is read when multiple read()s required in parse_debugger_output()
    - Run after send_command_impl()
    - Called inside send_command_mp_impl(), a.k.a. send_command()
*/
static int insert_output_end_marker(debugger_t *d)
{
    switch (d->index) {
        case (GDB_DEBUGGER):
            if (!send_command_impl(d, 3, "echo ", end_marker, "\n"))
                goto end_mark_err;
            break;
        case (PDB_DEBUGGER):
            if (!send_command_impl(d, 3, "p \"", end_marker, "\"\n"))
                goto end_mark_err;
            break;
    }

    return A_OK;
    
end_mark_err:
    pfemr ("Failed to insert debugger output end marker");
}

int send_command_mp_impl(debugger_t *d, int max_strs, ...)
{
    int str_count = 0, str_len;
    char *str;
    va_list strs;

    if (!d->running)
        pfemr("Debugger \"%s\" not running, unable to send command", d->title);

    int cmd_chars_left = DBG_CMD_SIZE - 1;
    d->last_cmd[0] = '\0';

    va_start(strs, max_strs);
    for (str = va_arg(strs, char*); str != NULL; str = va_arg(strs, char*)) {

        int str_len = strlen(str);

        // d->last_cmd
        if (str_len > cmd_chars_left)
            pfemr("Error: command string longer than buffer");
        strcat(d->last_cmd, str);
        cmd_chars_left -= str_len;

        if (++str_count > max_strs)
            break;

        if (write (d->stdin_pipe, str, str_len) == -1)
            pfemr_errno("Command write error on \"%s\"", str);
    }
    va_end(strs);

    if (!insert_output_end_marker(d))
        pfemr("Failed to insert debugger output end marker");

    if(!parse_debugger_output(d))
        pfemr("Failed to parse debugger output");

    return A_OK;
}

static int send_setup_commands(debugger_t *d)
{
    switch (d->index) {
        case GDB_DEBUGGER:
            if (!send_command_mp(d, "set confirm off\n"))
                pfemr("Failed to set confirm to off in GDB");
            break;
        case PDB_DEBUGGER:
            break;
    }

    return A_OK;
}

int start_debugger(debugger_t *d)
{
    if (!d->initialized)
        pfemr("Debugger not initialized");

    start_debugger_proc(d);

    if (!parse_debugger_output(d))
        pfemr("Failed to clear initial debugger output");

    if (!send_setup_commands(d))
        pfemr ("Failed to send setup commands");

    return A_OK;
}

int stop_debugger(debugger_t *d)
{
    if (close(d->stdin_pipe) == -1 || close(d->stdout_pipe) == -1)
        pfemr_errno("Failed to close debugger pipe");

    int wstatus;
    kill(d->pid, SIGTERM);
    waitpid(d->pid, &wstatus, 0);

    clear_buffer(d->data_buf);
    clear_buffer(d->cli_buf);
    clear_buffer(d->async_buf);
    clear_buffer(d->prog_buf);

    d->running = false;

    return A_OK;
}

int restart_debugger(debugger_t *d)
{
    if (!stop_debugger(d))
        pfemr("Failed to stop debugger");

    return start_debugger(d);
}

void destroy_debugger(debugger_t *d)
{
    if (d) {

        if (d->running)
            stop_debugger(d);

        destroy_command(d->command);
        destroy_buffer(d->data_buf);
        destroy_buffer(d->cli_buf);
        destroy_buffer(d->async_buf);
        destroy_buffer(d->prog_buf);

        free(d);
    }
}

/**********
  Get data
 **********/

int debugger_index(debugger_t *d)
{
    return d->index;
}
const char* debugger_title(debugger_t *d)
{
    return d->title;
}
char* last_command(debugger_t *d)
{
    return d->last_cmd;
}
