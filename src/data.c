/*
    Window data commands
    ---------
    - See ../tests/test_data.c for example usage
    - See ../misc/debugger_output.md for example debugger output
*/
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>

#include "data.h"
#include "error.h"
#include "debugger.h"
#include "utils.h"

#define BIN_PATH_BUF_SIZE 128
#define SRC_FUNC_BUF_SIZE 128
#define SRC_PATH_BUF_SIZE 256
#define SRC_LINE_BUF_SIZE 32
#define ASM_BUF_SIZE KB_TO_BYTES(64)
#define BREAK_BUF_SIZE KB_TO_BYTES(6)
#define DBG_OUT_BUF_SIZE KB_TO_BYTES(64)
#define LOC_VAR_BUF_SIZE KB_TO_BYTES(12)
#define PROG_OUT_BUF_SIZE KB_TO_BYTES(64)
#define REG_BUF_SIZE KB_TO_BYTES(64)
#define STACK_BUF_SIZE KB_TO_BYTES(12)
#define WATCH_BUF_SIZE KB_TO_BYTES(6)

enum {
    DEFAULT_UPDATE,
    
};

typedef struct watch_s {
    int index;
    char *var;
    struct watch_s *next;
} watch_t;

typedef struct data_s {

    // Misc
    buffer_t *prog_name_buf;
    buffer_t *curr_func_buf;
    buffer_t *curr_src_path_buf;
    buffer_t *curr_src_line_buf;

    // Window
    buffer_t *asm_buf;
    buffer_t *break_buf;
    buffer_t *dbg_out_buf;
    buffer_t *loc_vars_buf;
    buffer_t *prog_out_buf;
    buffer_t *reg_buf;
    buffer_t *stack_buf;
    buffer_t *watch_buf;

    time_t prog_update_time;
    watch_t *watchpoints;

} data_t;

char *none_val = "none";
char *empty_str = "";

/*
static void no_buff_data(buffer_t *buf)
{
    const char *no_data_str = "Not supported";
    cp_buf_str(buf, no_data_str);
}
*/

static void destroy_watchpoints(data_t *d)
{
    watch_t *w, *tw;
    w = d->watchpoints;
    while (w) {
        tw = w;
        w = w->next;
        free(tw->var);
        free(tw);
    }
    d->watchpoints = NULL;
}

void destroy_data(data_t *d)
{
    if (d) {
        destroy_buffer(d->prog_name_buf);
        destroy_buffer(d->curr_func_buf);
        destroy_buffer(d->curr_src_path_buf);
        destroy_buffer(d->curr_src_line_buf);
        destroy_buffer(d->asm_buf);
        destroy_buffer(d->break_buf);
        destroy_buffer(d->dbg_out_buf);
        destroy_buffer(d->loc_vars_buf);
        destroy_buffer(d->prog_out_buf);
        destroy_buffer(d->reg_buf);
        destroy_buffer(d->stack_buf);
        destroy_buffer(d->watch_buf);

        destroy_watchpoints(d);

        free(d);
    }
}

data_t* initialize_data (void)
{
    data_t *d;
    if (!(d = malloc(sizeof(data_t))))
        pfemn_errno("Failled to allocate data_t object");

    if (!(d->prog_name_buf = create_buffer(BIN_PATH_BUF_SIZE))) goto data_err;
    if (!(d->curr_func_buf = create_buffer(SRC_FUNC_BUF_SIZE))) goto data_err;
    if (!(d->curr_src_path_buf = create_buffer(SRC_PATH_BUF_SIZE))) goto data_err;
    if (!(d->curr_src_line_buf = create_buffer(SRC_LINE_BUF_SIZE))) goto data_err;

    if (!(d->asm_buf = create_buffer(ASM_BUF_SIZE))) goto data_err;
    if (!(d->break_buf = create_buffer(BREAK_BUF_SIZE))) goto data_err;
    if (!(d->dbg_out_buf = create_buffer(DBG_OUT_BUF_SIZE))) goto data_err;
    if (!(d->loc_vars_buf = create_buffer(LOC_VAR_BUF_SIZE))) goto data_err;
    if (!(d->prog_out_buf = create_buffer(PROG_OUT_BUF_SIZE))) goto data_err;
    if (!(d->reg_buf = create_buffer(REG_BUF_SIZE))) goto data_err;
    if (!(d->stack_buf = create_buffer(STACK_BUF_SIZE))) goto data_err;
    if (!(d->watch_buf = create_buffer(WATCH_BUF_SIZE))) goto data_err;

    d->watchpoints = NULL;

    return d;

data_err:
    destroy_data(d);
    pfemn("Failled to allocate data_t buffer");
}

/***********
  Utilities
 ***********/

/*
    Concatenate GDB/MI key value into buffer_t object
    --------
    - Advances source pointer past key value if it exists
    - Parameters:
        dest_buf:  destination buffer_t pointer
        src_ptr_adr:  pointer to address of source buffer
        key:  key string plus "=\"" before value
    - Example usage:
        concat_value_gdb(data->prog_name_buf, &src_ptr, "name=\"");
*/
static int concat_value_gdb(buffer_t *dest_buf, char **src_ptr_addr, const char *key)
{
    // Move to value
    char *sp = strstr(*src_ptr_addr, key);
    if (!sp)
        return FAIL;
    sp += strlen(key);

    bool is_string = false;
    bool is_arr = false;

    // Check for initial hex value
    if ( *sp      == '0' &&  
        *(sp + 1) == 'x') {

        // Empty string 0x0 -> none
        if (*(sp + 2) == '0' &&
            *(sp + 3) == '"') {
            concat_buf_str(dest_buf, "none");
            *src_ptr_addr = sp;
            return A_OK;
        } 

        // Skip hex address before string value
        else {  
            while (*sp++ != ' ') {  
                ;
            }
            is_string = true;
            ++sp;
        }
    } 

    if (*sp == '{') {
        is_arr = true;
    }

    while (true) {

        if (*sp == '\0') {
            break;
        }

        if (!is_string && !is_arr) {
            while (*sp != '\"' && *(sp + 1) != '\0') {
                concat_buf_ch(dest_buf, *sp++);
            }
            break;
        }

        // String after stripped hex address, array, struct
        if (*sp == '\\' && *(sp + 1) == '\"') {
            concat_buf_ch(dest_buf, '\"');
            if (*(sp + 2) == '\"') {
                break;
            } else {
                sp += 2;
            }
        } else if (*sp == '\\' && *(sp + 1) == '\\') {
            concat_buf_ch(dest_buf, '\\');
            sp += 2;
        } else {
            if (is_arr) {
                if (*sp == '}' && *(sp + 1) == '\"') {
                    concat_buf_ch(dest_buf, *sp++);
                    *src_ptr_addr = sp;
                    return A_OK;
                }
            } else {
                if (*sp == '\"' && *(sp + 1) == '\0') {
                    break;
                }
            }
            concat_buf_ch(dest_buf, *sp++);
        }
    }

    *src_ptr_addr = sp;

    return A_OK;
}

/***********
  Misc data
 ***********/

char* program_name(data_t *d)
{
    return buffer(d->prog_name_buf);
}
char* current_function(data_t *d)
{
    return buffer(d->curr_func_buf);
}
char* current_source_file_path(data_t *d)
{
    return buffer(d->curr_src_path_buf);
}
char* current_source_line(data_t *d)
{
    return buffer(d->curr_src_line_buf);
}

static void set_prog_update_time(data_t *data)
{
    struct stat file_stat;
    if (stat(program_name(data), &file_stat) == -1)
        file_stat.st_mtim.tv_sec = -1;
    data->prog_update_time = file_stat.st_mtim.tv_sec;
}

static bool prog_updated(data_t *data, debugger_t *dbg)
{
    struct stat file_stat;
    if (stat(program_name(data), &file_stat) == -1)
        return false;
    if (data->prog_update_time == -1)
        return true;
    return data->prog_update_time < file_stat.st_mtim.tv_sec;
}

int update_misc_data(data_t *data, debugger_t *dbg)
{
    char *cmd;
    switch (debugger_index(dbg)) {
        case GDB_DEBUGGER: cmd = "-thread-info\n"; break;
        case PDB_DEBUGGER: break;
    }

    if (!send_command_mp(dbg, cmd))
        pfemr("Failed to send misc data command");

    char *src_ptr = data_buffer(dbg);

    clear_buffer(data->prog_name_buf);
    clear_buffer(data->curr_func_buf);
    clear_buffer(data->curr_src_path_buf);
    clear_buffer(data->curr_src_line_buf);

    switch (debugger_index(dbg)) {

    case GDB_DEBUGGER:
        if (!strstr(src_ptr, "threads=[]")) {
            if (!concat_value_gdb(data->prog_name_buf, &src_ptr, "name=\""))
                pfemr("Failed to add program path data");
            if (!concat_value_gdb(data->curr_func_buf, &src_ptr, "func=\""))
                pfemr("Failed to add current function data");
            if (!concat_value_gdb(data->curr_src_path_buf, &src_ptr, "fullname=\""))
                pfemr("Failed to add current source path data");
            if (!concat_value_gdb(data->curr_src_line_buf, &src_ptr, "line=\""))
                pfemr("Failed to add current source line data");
            set_prog_update_time(data);
        }
        break;

    case PDB_DEBUGGER:
        break;
    }

    return A_OK;
}

/**********
  Assembly
 **********/

char* assembly_data(data_t *d)
{
    return buffer(d->asm_buf);
}

int update_assembly_data(data_t *data, debugger_t *dbg)
{
    char *cmd;
    switch (debugger_index(dbg)) {
        case GDB_DEBUGGER: cmd = "disassemble "; break;
        case PDB_DEBUGGER: break;
    }

    char *func = (current_function(data)[0] == '\0') ? "main" : current_function(data);

    if (send_command_mp(dbg, cmd, func, "\n") == FAIL)
        pfemr("Failed to send update assembly command");

    char *src_ptr = cli_buffer(dbg);
    buffer_t *dest_buf = data->asm_buf;

    clear_buffer(dest_buf);

    switch (debugger_index(dbg)) {
    
    case GDB_DEBUGGER:
        if (strstr(src_ptr, "error") == NULL) {
            while (*src_ptr != '\0') {
                if ( *src_ptr      == 'D' &&
                    *(src_ptr + 1) == 'u' &&
                    *(src_ptr + 2) == 'm' &&
                    *(src_ptr + 3) == 'p' &&
                    *(src_ptr + 4) == ' ' &&
                    *(src_ptr + 5) == 'o' &&
                    *(src_ptr + 6) == 'f') {

                    while (*src_ptr++ != '\n') {
                        ;
                    }
                } if ( *src_ptr      == 'E' &&
                      *(src_ptr + 1) == 'n' &&
                      *(src_ptr + 2) == 'd' &&
                      *(src_ptr + 3) == ' ' &&
                      *(src_ptr + 4) == 'o' &&
                      *(src_ptr + 5) == 'f') {

                    while (*src_ptr++ != '\n') {
                        ;
                    }
                } else if (*src_ptr == '\\' && isalpha(*(src_ptr + 1))) {
                    if (*(src_ptr + 1) == 'n') {
                        src_ptr += 2;
                    } else if (*(src_ptr + 1) == 't') {
                        src_ptr += 2;
                    } 
                } else {
                    concat_buf_ch(dest_buf, *src_ptr++);
                }
            }
        }
        break;

    case PDB_DEBUGGER:
        break;

    }

    return A_OK;
}

/*************
  Breakpoints
 *************/

char* breakpoint_data(data_t *d)
{
    return buffer(d->break_buf);
}

static int update_breakpoint_data(data_t *data, debugger_t *dbg)
{
    char *cmd;
    switch (debugger_index(dbg)) {
        case GDB_DEBUGGER: cmd = "-break-info\n"; break;
        case PDB_DEBUGGER: break;
    }

    if (!send_command_mp(dbg, cmd))
        pfemr("Failed to send breakpoint info command");

    bool parsing = true;
    char *src_ptr = data_buffer(dbg);
    buffer_t *dest_buf = data->break_buf;

    clear_buffer(dest_buf);

    while (parsing) {

        switch (debugger_index(dbg)) {

        case GDB_DEBUGGER:
            if ((src_ptr = strstr(src_ptr, "bkpt="))) {
                concat_buf_ch(dest_buf, '(');
                if (!concat_value_gdb(dest_buf, &src_ptr, "number=\""))
                    pfemr("Failed to get breakpoint index");
                concat_buf_str(dest_buf, ") ");
                if (!concat_value_gdb(dest_buf, &src_ptr, "original-location=\""))
                    pfemr("Failed to get breakpoint location");
                concat_buf_ch(dest_buf, '\n');
            } else {
                parsing = false;
            }
            break;

        case PDB_DEBUGGER:
            break;
        }
    }

    return A_OK;
}

int add_breakpoint(data_t *data, debugger_t *dbg, char *break_str)
{
    char *cmd;
    switch (debugger_index(dbg)) {
        case GDB_DEBUGGER: cmd = "-break-insert "; break;
        case PDB_DEBUGGER: break;
    }
    if (!send_command_mp(dbg, cmd, break_str, "\n"))
        pfemr("Failed to send add breakpoint command");
    if (!update_breakpoint_data(data, dbg))
        pfemr("Failed to update breakpoint data");
    return A_OK;
}

int delete_breakpoint(data_t *data, debugger_t *dbg, char *break_index)
{
    char *cmd;
    switch (debugger_index(dbg)) {
        case GDB_DEBUGGER: cmd = "-break-delete "; break;
        case PDB_DEBUGGER: break;
    }
    if (!send_command_mp(dbg, cmd, break_index, "\n"))
        pfemr("Failed to send delete breakpoint command");
    if (!update_breakpoint_data(data, dbg))
        pfemr("Failed to update breakpoint data");
    return A_OK;
}

int delete_all_breakpoints(data_t *data, debugger_t *dbg)
{

    char *cmd;
    switch (debugger_index(dbg)) {
        case GDB_DEBUGGER: cmd = "-break-delete\n"; break;
        case PDB_DEBUGGER: break;
    }

    if (!send_command_mp(dbg, cmd))
        pfemr("Failed to send delete all breakpoints command");

    if (!update_breakpoint_data(data, dbg))
        pfemr("Failed to update breakpoint data");

    return A_OK;
}

/*********************
  Debugger CLI output
 *********************/

char* debugger_cli_output_data(data_t *d)
{
    return buffer(d->dbg_out_buf);
}

int update_debugger_cli_output_data(data_t *data, debugger_t *dbg)
{
    char *src_ptr = cli_buffer(dbg);
    buffer_t *dest_buf = data->dbg_out_buf;

    concat_buf_ch(dest_buf, '(');
    concat_buf_str(dest_buf, debugger_title(dbg));
    concat_buf_str(dest_buf, ") ");
    concat_buf_str(dest_buf, last_command(dbg));
    concat_buf_ch(dest_buf, '\n');

    while (*src_ptr != '\0') {

        //  \\\t, \\\n
        if (*src_ptr == '\\' && isalpha(*(src_ptr + 1))) {
            if (*(src_ptr + 1) == 'n') {
                src_ptr += 2;
            } else if (*(src_ptr + 1) == 't') {
                src_ptr += 2;
            } 
        }

        concat_buf_ch(dest_buf, *src_ptr++);
    }

    return A_OK;
}

/*****************
  Local Variables
 *****************/

char* local_variable_data(data_t *d)
{
    return buffer(d->loc_vars_buf);
}

int update_local_variable_data(data_t *data, debugger_t *dbg)
{
    char *cmd;
    char *src_ptr = data_buffer(dbg);
    switch (debugger_index(dbg)) {
        case GDB_DEBUGGER: cmd = "-stack-list-locals 1\n"; break;
        case PDB_DEBUGGER: break;
    }
    if (!send_command_mp(dbg, cmd))
        pfemr("Failed to send local variable info command");

    int index = 1;
    char index_buf[4];
    buffer_t *dest_buf = data->loc_vars_buf;
    clear_buffer(dest_buf);

    switch (debugger_index(dbg)) {

    case GDB_DEBUGGER:
        while ((src_ptr = strstr(src_ptr, "name="))) {
            concat_buf_ch(dest_buf, '(');
            snprintf(index_buf, sizeof(index_buf), "%d", index++);
            concat_buf_str(dest_buf, index_buf);
            concat_buf_str(dest_buf, ") ");
            if (!concat_value_gdb(dest_buf, &src_ptr, "name=\""))
                pfemr("Failed to allocate local variable name");
            concat_buf_str(dest_buf, ": ");
            if (!concat_value_gdb(dest_buf, &src_ptr, "value=\""))
                pfemr("Failed to allocate local variable value");
            concat_buf_ch(dest_buf, '\n');
        }
        break;

    case PDB_DEBUGGER:
        break;

    }

    return A_OK;
}

/****************
  Program output
 ****************/

char* program_output_data(data_t *d)
{
    return buffer(d->prog_out_buf);
}

void update_program_output_data(data_t *data, debugger_t *dbg)
{
    concat_buf_str(data->prog_out_buf, program_buffer(dbg));
}

/*************
  Watchpoints
 *************/

char* watchpoint_data(data_t *d)
{
    return buffer(d->watch_buf);
}

static int update_watchpoint_data(data_t *data, debugger_t *dbg)
{
    char *cmd;
    switch (debugger_index(dbg)) {
        case GDB_DEBUGGER: cmd = "-data-evaluate-expression "; break;
        case PDB_DEBUGGER: break;
    }

    buffer_t *dest_buf = data->watch_buf;
    watch_t *curr_watch = data->watchpoints;

    clear_buffer(dest_buf);

    while (curr_watch) {

        if (!send_command_mp(dbg, cmd, curr_watch->var, "\n"))
            pfemr("Failed to send watchpoint info command (var=\"%s\")", curr_watch->var);

        char idx_buf[8];
        char *src_ptr = data_buffer(dbg);

        switch (debugger_index(dbg)) {

            case GDB_DEBUGGER:

                // Index
                concat_buf_ch(dest_buf, '(');
                snprintf(idx_buf, sizeof(idx_buf), "%d", curr_watch->index);
                concat_buf_str(dest_buf, idx_buf);
                concat_buf_str(dest_buf, ") ");

                // Variable
                concat_buf_str(dest_buf, curr_watch->var);
                concat_buf_str(dest_buf, ": ");

                // Value
                if ((src_ptr = strstr(src_ptr, "done"))) {
                    if (!concat_value_gdb(dest_buf, &src_ptr, "value=\""))
                        pfemr("Failed to get watchpoint value (var=\"%s\")", curr_watch->var);
                    concat_buf_ch(dest_buf, '\n');
                } else {
                    concat_buf_str(dest_buf, "none\n");
                }

                break;

            case PDB_DEBUGGER:
                break;
        }

        curr_watch = curr_watch->next;
    }

    return A_OK;
}

int add_watchpoint(data_t *data, debugger_t *dbg, char *var_str)
{
    int index = 1;
    watch_t *curr_watch = data->watchpoints, 
            *prev_watch = NULL;

    // Add watch_t object
    if (!data->watchpoints) {
        if (!(data->watchpoints = malloc(sizeof(watch_t))))
            goto add_watch_t_err;
        curr_watch = data->watchpoints;
    } else {
        while (curr_watch) {
            if (strcmp(var_str, curr_watch->var) == 0)
                return A_OK;
            prev_watch = curr_watch;
            curr_watch = curr_watch->next;
        }
        if (!(prev_watch->next = malloc(sizeof(watch_t))))
            goto add_watch_t_err;
        curr_watch = prev_watch->next;
    }

    // Add value
    curr_watch->var = malloc(strlen(var_str) + 1);
    strcpy(curr_watch->var, var_str);
    curr_watch->next = NULL;

    // Re-index
    int idx = 1;
    curr_watch = data->watchpoints;
    while (curr_watch) {
        curr_watch->index = idx++;
        prev_watch = curr_watch;
        curr_watch = curr_watch->next;
    }

    // Update data
    if (!update_watchpoint_data(data, dbg))
        pfemr("Failed to update watchpoint data");

    return A_OK;

add_watch_t_err:
    pfemr_errno ("Failed to allocate watch_t object (index=%d, var=\"%s\")", index, var_str);
}

int delete_watchpoint (data_t *data, debugger_t *dbg, char *index)
{
    watch_t *curr_watch = data->watchpoints,
            *prev_watch = data->watchpoints,
            *tmp_watch = NULL;
    bool watch_deleted = false;

    while (curr_watch) {

        // Index found, delete
        if (curr_watch->index == atoi(index)) {
            tmp_watch = curr_watch;
            if (curr_watch == data->watchpoints)
                data->watchpoints = curr_watch->next;
            else
                prev_watch->next = curr_watch->next;
            if (tmp_watch->var)
                free (tmp_watch->var);
            free (tmp_watch);
            watch_deleted = true;
            break;
        }

        prev_watch = curr_watch;
        curr_watch = curr_watch->next;
    }

    if (!watch_deleted)
        return A_OK;

    // Re-index
    curr_watch = data->watchpoints;
    int idx = 1;
    while (curr_watch) {
        curr_watch->index = idx++;
        prev_watch = curr_watch;
        curr_watch = curr_watch->next;
    }

    // Update data
    if (!update_watchpoint_data(data, dbg))
        pfemr("Failed to update watchpoint data");

    return A_OK;
}

int delete_all_watchpoints (data_t *data, debugger_t *dbg)
{
    destroy_watchpoints(data);

    if (!update_watchpoint_data(data, dbg))
        pfemr("Failed to update watchpoint data");

    return A_OK;
}

/*****************
  Update all data
 *****************/

int update_all_data(data_t *data, debugger_t *dbg)
{
    if (prog_updated(data, dbg))
        if (!restart_debugger(dbg))
            pfemr("Failed to restart debugger");

    update_misc_data(data, dbg);
    update_assembly_data(data, dbg);
    update_breakpoint_data(data, dbg);
    update_debugger_cli_output_data(data, dbg);
    update_local_variable_data(data, dbg);
    update_watchpoint_data(data, dbg);

    return A_OK;
}
