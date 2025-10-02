
# To Do, Fix, ...

TODO: Decouple, reorganize code
TODO: Replace int return values with enums, separate values for each module
TODO: Change switch statement formatting
  case a:
    func1();
  break:
  case b:
TODO: tui, [A-Za-z] is part of interface, { type: "command" or "window"; ... }
TODO: Convert enum values to typedef enums
TODO: Use less heap
TODO: Implement libunwind error logging (?)
TODO: Implement file logging for details, reduce user error messages, maybe create termfu_logs binary
TODO: Compare binary and current source file times (manually or with gdb/mi)
FIX: stdout not printing (e.g. test_debugger.c)
TODO: Replace switch tables with dynamic dispatching and lookup table
TODO: Move descriptions next to variables in data.h
TODO: Switch to mmap for file processing
TODO: Create plugin type boolean functions (is window, form?), replace switch sections
TODO: Add example gdb/mi output for all functions
TODO: Add command history (persisted, limit)
BUG: Figure out startup bug where doesn't always load debugger
FIXME: Wait for debugger process to start before updating window data
TODO: Add function benchmarks, history
PERF: store dereferences in variables (?)
TODO: Add termfu tips back in (config copy, vim breakpoints, ...)
TODO: Add ">>> <cmd>" to Dbg output
FIXME: Fix make plugins
FIXME: Fix C++ class watches, add C++ test programs
FIXME: Fix valgrind leaks
TODO: Switch to semantic versioning, add release link to CONTRIBUTING.md, add RELEASE.md, patch versioning (v1.2.3 -> v1.2)
TODO: Create apt, dnf, pacman packages
