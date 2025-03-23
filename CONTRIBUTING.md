# Contributing

## General Guidelines
- Bug fixes, optimizations, new debugger implementations, and plugins are welcome.
- Use existing code conventions.

## Developer notes
- Run `make help` to print all scripts.
- Run `make todo` to print all source file tags, such as `TODO`, `FIX`, etc.
- Run `make build` to compile all test programs.
- The `logd()` function in `src/utilities.h` allows for `printf()`-style debugging when running `ncurses` by outputting to `debug.out`.
- It is recommended to create a shortcut for refreshing your terminal screen, as `ncurses` will make a mess of it when not shut down properly.

## Debugging `termfu_dev` with `termfu`
- Run `$ make debug` to start the `termfu` debugger. This requires `termfu` to be in your path.
- Open a separate terminal the same size as your debugger's terminal for the debugged process and run `make debugged`. This command gets the terminal's file (e.g. `/dev/pts/12`), creates a `GDB` command out of it (e.g. `tty /dev/pts/12`), and copies it into your clipboard. This command will instruct your debugger in the other terminal to send the debugged process's output to this terminal as well as allow you to send keyboard commands to the debugged process while inside this terminal.
- Back in the debugger terminal, open the prompt window, copy the command now in your clipboard into the prompt, and hit `Enter`.
- Set a breakpoint and run the program.
- Adjust `configs/.termfu_debugger` and `configs/.termfu_debugged` as needed
- You can use `GDB` as the debugger without `termfu` as the frontend using this same process.

<br><br>
