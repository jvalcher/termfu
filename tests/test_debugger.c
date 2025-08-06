#include "test.h"
#include "../src/debugger.h"
#include "../src/error.h"

int passed, failed;

static void print_debugger_buffers(debugger_t *d)
{
    printf("CLI_BUFFER:\n------- \n%s\n\n", cli_buffer(d));
    printf("DATA_BUFFER:\n------- \n%s\n\n", data_buffer(d));
    printf("ASYNC_BUFFER:\n------- \n%s\n\n", async_buffer(d));
    printf("PROGRAM_BUFFER:\n------- \n%s\n\n", program_buffer(d));
}

static void run_test_start_debugger(void)
{
    debugger_t *d = NULL;
    char **command = NULL;

    // Command
    command = create_command("gdb", "--interpreter=mi", "test_programs/hello");
    test_cond(command != NULL, " ");
    destroy_command(command);

    // Incorrect index
    command = create_command("gdb", "--interpreter=mi", "test_programs/hello");
    test_cond((d = init_debugger(123, command)) == NULL, " ");

    // Destory NULL debugger_t
    destroy_debugger(d);

    // gdb
    command = create_command("gdb", "--interpreter=mi", "test_programs/hello");
    test_cond((d = init_debugger(GDB_DEBUGGER, command)) != NULL, " ");
    test_cond(start_debugger(d) == A_OK, " ");
    test_cond(stop_debugger(d) == A_OK, " ");
    destroy_debugger(d);

    // pdb
    command = create_command("python", "-m", "pdb", "test_programs/gcd.py");
    test_cond((d = init_debugger(PDB_DEBUGGER, command)) != NULL, " ");
    test_cond(start_debugger(d) == A_OK, " ");
    test_cond(stop_debugger(d) == A_OK, " ");
    destroy_debugger(d);
}

static void run_test_debugger_buf(void)
{
    debugger_t *d = NULL;
    char **command = NULL;

    // gdb
    command = create_command("gdb", "--interpreter=mi", "test_programs/hello");
    test_cond((d = init_debugger(GDB_DEBUGGER, command)) != NULL, " ");
    test_cond(start_debugger(d) == A_OK, " ");

    printf("\n#########\nGDB OUT:\n#########\n\n");
    test_cond(send_command(d, "-break-insert main\n") == A_OK, " ");
    print_debugger_buffers(d);
    test_cond(send_command(d, "-exec-run\n") == A_OK, " ");
    printf("-exec-run\n-------\n");
    print_debugger_buffers(d);
    test_cond(send_command(d, "-exec-continue\n") == A_OK, " ");
    printf("-exec-continue\n-------\n");
    print_debugger_buffers(d);

    destroy_debugger(d);
    printf("\n\n");

    // pdb
    command = create_command("python", "-m", "pdb", "test_programs/gcd.py");
    test_cond((d = init_debugger(PDB_DEBUGGER, command)) != NULL, " ");
    test_cond(start_debugger(d) == A_OK, " ");

    printf("\n#########\nPDB OUT:\n#########\n\n");
    test_cond(send_command(d, "break main\n") == A_OK, " ");
    print_debugger_buffers(d);
    test_cond(send_command(d, "continue\n") == A_OK, " ");
    print_debugger_buffers(d);
    test_cond(send_command(d, "continue\n") == A_OK, " ");
    print_debugger_buffers(d);

    destroy_debugger(d);
}

int main(void)
{
    test_init();

    run_test_start_debugger();
    run_test_debugger_buf();

    test_results();

    return 0;
}
