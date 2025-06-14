# Contributing

## General Guidelines
- Bug fixes, optimizations, new debugger implementations, and plugins are welcome.
- Use existing code conventions.

## Developer notes
- Run `make help` to print all scripts.
- Run `make todo` to print all source file tags.
  - Supported tags: TODO, FIXME, BUG, PERF, HACK, NOTE, TEST, XXX 
- Run `make build` to compile all test programs.
- The `logd()` function in `src/utilities.h` allows for `printf()`-style debugging when running `ncurses` by outputting to `debug.out`.
- It is recommended to create a shortcut for refreshing your terminal screen, as `ncurses` will make a mess of it when not shut down properly.
- This program uses `ncurses6`.

## Testing
- Run `$ make test t=tests/<test>.c` to create the test binary `termfu_test`.
- Begin all test source file names with `test_`.
- All test utilities go in `tests/test_utilties.c`.

## Debugging `termfu_dev` or `termfu_test`.
- Run `$ make debug` to debug with `termfu` or just use regular `GDB`. `$ make debug` requires `termfu` to be in your path.
- Open a separate terminal the same size as your debugger's terminal for the debugged process and run `make debugged`. This command gets the terminal's file (e.g. `/dev/pts/12`), creates a `GDB` command out of it (e.g. `tty /dev/pts/12`), and copies it into your clipboard. It then puts the terminal to sleep, which allows you to send keyboard commands to the debugged process while inside this terminal.
- Back in the debugger terminal, copy the command now in your clipboard into the prompt and hit `Enter`. This tells the debugger to send the debugged process's output to the other terminal.
- Set a breakpoint and run the program.
- Adjust `configs/.termfu_debugger` and `configs/.termfu_debugged` as needed

<br><br>
