/*
    Debugger subprocess interface
    --------
    - Supports: 
        - GDB
        - python PDB
    - See ../tests/test_debugger.c for example usage
*/
#ifndef debugger_h
#define debugger_h

#include <stdbool.h>
#include <sys/stat.h>
#include "utils.h"

#define DBG_MAX_STRS  20

enum { 
    GDB_DEBUGGER, 
    PDB_DEBUGGER 
};    

typedef struct debugger debugger_t;

/*
    Command array
    ---------
    - Convert maximum of <DBG_MAX_STRS> command strings into array to be run by execvp()
    - Deallocate with destroy_command() or destroy_debugger()
    - Example usage:
        char **cmd = create_command("gdb", "--interpreter=mi", prog_path);
        cmd == {"gdb", "--interpreter=mi", prog_path, NULL}
*/
char** create_command_impl(int max_strs, ...);
#define create_command(...) create_command_impl(DBG_MAX_STRS, __VA_ARGS__, NULL)

/*
    Deallocate command array
    ---------
    - Also deallocated by destroy_debugger()
*/
void destroy_command(char** command);

/*
    debugger_t object
    --------
    - Use create_command() to allocate execvp() array
    - Parameters:
        dbg_index: *_DEBUGGER enum
        command:   e.g. {"gdb", "--interpreter=mi", "a.out", NULL}
    - Returns:  
        - Pointer to debugger_t object
*/
debugger_t* init_debugger(int dbg_index, char** command);

/*
    Deallocate debugger_t object
*/
void destroy_debugger(debugger_t *d);

/*
    Debugger subprocess
*/
int start_debugger(debugger_t *d);  //-> A_OK, FAIL
int stop_debugger(debugger_t *d);
int restart_debugger(debugger_t *d);

/*
    Send debugger command
    ----------
    - Combine maximum of <DBG_MAX_STRS> separate command strings
    - Final string must end with '\n' 
    - send_command_mp() 
        - Send command, insert end-of-output marker, parse data into buffers
    - send_command()
        - Send command only
        - Useful for sending multiple commands before parsing
        - e.g. flushing program stdout in GDB:
            send_command(dbg, "-exec-until ", line_num, "\n");
            send_command_mp(dbg, "call ((void(*)(int))fflush)(0)\n");
*/
int send_command_impl(debugger_t *dbg, int max_strs, ...);  //-> A_OK, FAIL
#define send_command(d,...) send_command_impl((d), DBG_MAX_STRS, __VA_ARGS__, NULL)
    //
int send_command_mp_impl(debugger_t *dbg, int max_strs, ...);
#define send_command_mp(d,...) send_command_mp_impl((d), DBG_MAX_STRS, __VA_ARGS__, NULL)

/*******
  Flags
 *******/

void set_raw_output(debugger_t *d);

/******
  Data
 ******/

/*
    Misc 
*/
int debugger_index(debugger_t *d);          //-> <*_DEBUGGER> enum index
const char* debugger_title(debugger_t *d);  //-> Title string, e.g. "gdb"
char* last_command(debugger_t *d);          //-> Last run command string

/*
    Output data buffers
    -----------
    GDB:
        - cli_buffer - Debugger CLI output
        - data_buffer - Data values e.g. source file path, line number, breakpoints
        - async_buffer - Async state change info e.g. started, stopped
        - program_buffer - Program output

    PDB:
        - cli_buffer - Debugger CLI output
        - program_buffer - Program output
*/
char* cli_buffer(debugger_t *d);
char* data_buffer(debugger_t *d);
char* async_buffer(debugger_t *d);
char* program_buffer(debugger_t *d);

#endif
