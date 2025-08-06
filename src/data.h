/*
    Interact with data of debugged program
    --------
    - See ../tests/test_data.c for usage
*/
#ifndef DATA_H
#define DATA_H

#include "debugger.h"

typedef struct data_s data_t;

/*
    data_t
*/
data_t* initialize_data(void);
void destroy_data(data_t *d);
int update_all_data(data_t *data, debugger_t *dbg);  //-> A_OK, FAIL

/*
    Misc data
*/
char* program_name(data_t *d);
char* current_source_file_path(data_t *d);
char* current_source_line(data_t *d);
char* current_function(data_t *d);
int update_misc_data(data_t *data, debugger_t *dbg);  //-> A_OK, FAIL

/*
    Assembly
*/
char* assembly_data(data_t *d);
int update_assembly_data(data_t *data, debugger_t *dbg);  //-> A_OK, FAIL

/*
    Breakpoints
*/
char* breakpoint_data(data_t* d);
int add_breakpoint(data_t *data, debugger_t *dbg, char *break_str);  //-> A_OK, FAIL
int delete_breakpoint(data_t *data, debugger_t *dbg, char *break_index);
int delete_all_breakpoints(data_t *data, debugger_t *dbg);

/*
    Debugger CLI output
*/
char* debugger_cli_output_data(data_t *d);
int update_debugger_cli_output_data(data_t *data, debugger_t *dbg);  //-> A_OK, FAIL

/*
    Local variables
*/
char* local_variable_data(data_t *d);
int update_local_variable_data(data_t *data, debugger_t *dbg);

/*
    Program output
*/
char* program_output_data(data_t *d);
void update_program_output_data(data_t *data, debugger_t *dbg);

/*
    Watchpoints
*/
char* watchpoint_data(data_t *d);
int add_watchpoint(data_t *data, debugger_t *dbg, char *var_str);  //-> A_OK, FAIL
int delete_watchpoint (data_t *data, debugger_t *dbg, char *watch_index);
int delete_all_watchpoints (data_t *data, debugger_t *dbg);

#endif
