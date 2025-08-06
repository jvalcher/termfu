#include <libgen.h>

#include "test.h"
#include "../src/error.h"
#include "../src/data.h"
#include "../src/debugger.h"

int passed = 0, failed = 0;

const char *gdb_prog_name = "hello",
           *gdb_prog_src = "hello.c";

static void print_misc_data(data_t *d)
{
    printf("Program name: \"%s\"\n"
           "Current source path: \"%s\"\n"
           "Current source line: \"%s\"\n"
           "Current function: \"%s\"\n",
            program_name(d), 
            current_source_file_path(d), 
            current_source_line(d), 
            current_function(d));
}

static void run_test_misc_data_gdb(data_t *data, debugger_t *dbg)
{
    restart_debugger(dbg);

    // Program not running
    send_command_mp(dbg, "kill\n");
    update_misc_data(data, dbg);
    //print_misc_data(data);
    test_strs_eq(program_name(data), "", " ");
    test_strs_eq(current_source_file_path(data), "", " ");
    test_strs_eq(current_source_line(data), "", " ");
    test_strs_eq(current_function(data), "", " ");

    // Program running
    add_breakpoint(data, dbg, "main");
    send_command_mp(dbg, "-exec-run\n");
    send_command_mp(dbg, "-exec-until 23\n");
    update_misc_data(data, dbg);
    //print_misc_data(data);
    test_strs_eq(program_name(data), "hello", 
            "\"%s\" != \"%s\"", program_name(data), "hello");
    test_strs_eq(basename(current_source_file_path(data)), "hello.c", 
            "\"%s\" != \"%s\"", basename(current_source_file_path(data)), "hello.c");
    test_strs_eq(current_source_line(data), "23", 
            "\"%s\" != \"%s\"", current_source_line(data), "23");
    test_strs_eq(current_function(data), "main", 
            "\"%s\" != \"%s\"", current_function(data), "main"); 

    test_cond(delete_all_breakpoints(data, dbg) == A_OK, " ");
    test_cond(delete_all_watchpoints(data, dbg) == A_OK, " ");
}

static void run_test_assembly_gdb(data_t *data, debugger_t *dbg)
{
    restart_debugger(dbg);

    add_breakpoint(data, dbg, "main");
    send_command_mp(dbg, "-exec-run\n");
    update_assembly_data(data, dbg);
    test_strs_neq(assembly_data(data), "", " ");
    //printf("ASSEMBLY\n-----------\n%s\n\n", assembly_data(data));

    test_cond(delete_all_breakpoints(data, dbg) == A_OK, " ");
    test_cond(delete_all_watchpoints(data, dbg) == A_OK, " ");
}

static void run_test_breakpoints_gdb(data_t *data, debugger_t *dbg)
{
    restart_debugger(dbg);

    // None
    //printf("NONE\n--------\n%s\n\n", breakpoint_data(data));
    test_strs_eq(breakpoint_data(data), "", "\"%s\" != \"%s\"", breakpoint_data(data), "");

    // Single
    test_cond(add_breakpoint(data, dbg, "main") == A_OK, " ");
    const char *single_str = "(1) main\n";
    //printf("SINGLE\n--------\n%s\n\n", breakpoint_data(data));
    test_strs_eq(breakpoint_data(data), single_str, "\"%s\" != \"%s\"", breakpoint_data(data), single_str);

    // Multiple
    const char *mult_str = "(1) main\n(2) hello2.c:11\n(3) hello.c:23\n";
    test_cond(add_breakpoint(data, dbg, "hello2.c:11") == A_OK, " ");
    test_cond(add_breakpoint(data, dbg, "hello.c:23") == A_OK, " ");
    //printf("MULTIPLE\n--------\n%s\n\n", breakpoint_data(data));
    test_strs_eq(breakpoint_data(data), mult_str, "\n\"%s\" \n!=\n\"%s\"", breakpoint_data(data), mult_str);

    // Delete single
    const char *del_single_str = "(1) main\n(3) hello.c:23\n";
    test_cond(delete_breakpoint(data, dbg, "2") == A_OK, " ");
    //printf("DELETE SINGLE\n--------\n%s\n\n", breakpoint_data(data));
    test_strs_eq(breakpoint_data(data), del_single_str, "\"%s\" != \"%s\"", breakpoint_data(data), del_single_str);

    // Delete all
    test_cond(delete_all_breakpoints(data, dbg) == A_OK, " ");
    //printf("DELETE ALL\n--------\n%s\n\n", breakpoint_data(data));
    test_strs_eq(breakpoint_data(data), "", "\"%s\" != \"%s\"", breakpoint_data(data), "");

    test_cond(delete_all_breakpoints(data, dbg) == A_OK, " ");
    test_cond(delete_all_watchpoints(data, dbg) == A_OK, " ");
}

static void run_test_debugger_cli_output_gdb(data_t *data, debugger_t *dbg)
{
    char *buf_ptr;
    restart_debugger(dbg);

    // Confirm command
    update_debugger_cli_output_data(data, dbg);
    buf_ptr = debugger_cli_output_data(data);
    const char *conf_cmd = "(gdb) set confirm off\n\n";
    //printf("%s", buf_ptr);
    test_strs_eq(buf_ptr, conf_cmd, "\n\"%s\"\n !=\n\"%s\"", buf_ptr, conf_cmd);

    // Breakpoint at main
    send_command_mp(dbg, "-break-insert main\n");
    send_command_mp(dbg, "-exec-run\n");
    update_debugger_cli_output_data(data, dbg);
    //printf("%s\n", debugger_cli_output_data(data));

    test_cond(delete_all_breakpoints(data, dbg) == A_OK, " ");
    test_cond(delete_all_watchpoints(data, dbg) == A_OK, " ");
}

static void run_test_local_vars_gdb(data_t *data, debugger_t *dbg)
{
    restart_debugger(dbg);

    send_command_mp(dbg, "-break-insert main\n");
    send_command_mp(dbg, "-exec-run\n");
    send_command_mp(dbg, "-exec-until 23\n");
    test_cond(update_local_variable_data(data, dbg) == A_OK, " ");
    //printf("LOCAL VARIABLES\n---------\n%s\n", local_variable_data(data));

    test_cond(delete_all_breakpoints(data, dbg) == A_OK, " ");
    test_cond(delete_all_watchpoints(data, dbg) == A_OK, " ");
}

static void run_test_program_output_gdb(data_t *data, debugger_t *dbg)
{
    restart_debugger(dbg);

    add_breakpoint(data, dbg, "main");
    send_command(dbg, "-exec-run\n");
    send_command(dbg, "-exec-until 24\n");
    send_command_mp (dbg, "call ((void(*)(int))fflush)(0)\n");
    update_program_output_data(data, dbg);
    //printf("PROGRAM OUTPUT\n---------\n%s\n", program_output_data(data));

    test_cond(delete_all_breakpoints(data, dbg) == A_OK, " ");
    test_cond(delete_all_watchpoints(data, dbg) == A_OK, " ");
}

static void run_test_watchpoints_gdb(data_t *data, debugger_t *dbg)
{
    restart_debugger(dbg);

    // Not in scope
    add_breakpoint(data, dbg, "main");
    send_command_mp(dbg, "-exec-run\n");
    test_cond(add_watchpoint(data, dbg, "str3") == A_OK, " ");
    const char *not_in_scope = "(1) str3: none\n";
    //printf("NOT IN SCOPE\n--------\n%s\n\n", watchpoint_data(data));
    test_strs_eq(watchpoint_data(data), not_in_scope, "\"%s\" != \"%s\"", watchpoint_data(data), not_in_scope);
    delete_watchpoint(data, dbg, "1");

    // In scope
    send_command_mp(dbg, "-exec-until 23\n");
    test_cond(add_watchpoint(data, dbg, "str3") == A_OK, " ");
    const char *in_scope = 
        "(1) str3: \"This is a long line of \\\"characters\\\" that might not fit in the window\"\n";
    //printf("IN SCOPE\n--------\n%s\n\n", watchpoint_data(data));
    test_strs_eq(watchpoint_data(data), in_scope, "\n\"%s\" \n!= \n\"%s\"", watchpoint_data(data), in_scope);

    // Number, number array
    const char *num_arr_str = 
        "(1) str3: \"This is a long line of \\\"characters\\\" that might not fit in the window\"\n"
        "(2) n1: 13\n"
        "(3) n_arr: {1, 2, 3, 4, 5}\n";
    test_cond(add_watchpoint(data, dbg, "n1") == A_OK, " ");
    test_cond(add_watchpoint(data, dbg, "n_arr") == A_OK, " ");
    //printf("NUMBER, NUMBER ARRAY\n--------\n%s\n\n", watchpoint_data(data));
    test_strs_eq(watchpoint_data(data), num_arr_str, "\n\"%s\" \n!= \n\"%s\"", watchpoint_data(data), num_arr_str);

    test_cond(delete_all_breakpoints(data, dbg) == A_OK, " ");
    test_cond(delete_all_watchpoints(data, dbg) == A_OK, " ");
}

static void run_all_data_tests(void)
{
    char **cmd;
    data_t *data;
    debugger_t *dbg;

    test_init();

    // GDB
    data = initialize_data();
    cmd = create_command("gdb", "--interpreter=mi", "test_programs/hello");
    dbg = init_debugger(GDB_DEBUGGER, cmd);
    if (!start_debugger(dbg))
        pfeme("Failed to start debugger");

    run_test_misc_data_gdb(data, dbg);
    run_test_assembly_gdb(data, dbg);
    run_test_breakpoints_gdb(data, dbg);
    run_test_debugger_cli_output_gdb(data, dbg);
    run_test_local_vars_gdb(data, dbg);
    run_test_program_output_gdb(data, dbg);
    run_test_watchpoints_gdb(data, dbg);

    destroy_data(data);
    destroy_debugger(dbg);

    // PDB

    test_results();
}

int main(void)
{
    run_all_data_tests();

    return 0;
}
