
# termIDE

A stress-free terminal development environment

- Fast performance
- Fast navigation
- Simple configuration
- Multiple custom window layouts
- Built-in plugin creator
- Documented within an inch of its life



# Configuration file (.termide)

## Examples

### Layouts:

- see `_Plugins.md` for a list of plugin codes

- layout header format:

  `[[ <type> : <title string> ]]`

- sections:
    >p  plugin, key, title settings
    >h  header actions layout
    >w  window layout for output plugins

- plugin (>p) setting format:

  `<code> : <key> : <window/action title string>`

- Example layout configuration:

  ```
  [ layout : gdb_debug_1 ]
  >p
  Bld : b : (b)uilds -
  Lay : l : (l)ayouts -
  Qut : q : (q)uit
  Run : r : (r)un
  Nxt : n : (n)ext
  Stp : s : (s)tep
  Con : c : (c)ontinue
  Fin : f : (f)inish
  Kil : k : (k)ill
  Brk : b : (b)reakpoints
  Src : f : source (f)ile
  Wat : w : (w)atch
  Loc : l : (l)ocal vars
  Out : o : (o)utput
  >h
  b l q
  r n s c f k
  >w
  b f f
  w f f
  l o o
  ```



## To do


- config file name can be ".termide*" 

  - plugin or title to indentify file being used


- debug macro change

  ```
  #ifdef DEBUG
  #define dprintf(...) fprintf(stderr, __VA_ARGS__)
  #else
  #define dprintf(...) ((void)0)
  #endif
  ```


- Undo/redo

  - On everything
  - All events combined in D_LL whether gdb or other actions


- software builds

  - list bash commands under each [build_01] config heading
  - runs first build in config file by default


- print available commands in current window
  - `?`


- syntax highlighting
  - vim - .termide file
  - source code



-------
PLUGINS
-------

- misc functionality

  - codes == three alphabet characters == 52³ == 140,608
  - build from source in plugins/ or use library files

  - source organization
    - plugins/
      - \_inc/
      - \_lib/
      - \_plugins.h
      - \_plugins.c
      - gdb_window_watch.c
      - ...

    - config file
      - spaces ok in header
      - two brackets for header categories with label, etc. after ":"

      - global plugin-to-key
        - code : key : title string
        - titles can be blank

        [plugins]
        Wat : a : w(a)tch
        Reg : g : re(g)isters
        Src : s : (s)ource
        Lsc : l :

      - layout
        - >p - custom plugin-to-key settings
        - >a - actions, displayed in header by title
        - >w - window layout

        [[ layout : gdb ]]
        >p
        Wat : w : (w)atch
        Reg : r : (r)egisters
        >a
        a b c
        d e f
        >w
        b s s
        w s s
        r o m

  - separate functionality, config file ($HOME/.config) that preserves custom plugin
    functionality while allowing updates

  - selected item output
    - utility function to display selected element info in separate window
    - select:
      - breakpoint, variable, array, etc.
    - output to:
      - existing window
        - switch to different window with arrow keys
      - popup window


- plugin ideas

  - gdb
  - memory viewer
    - color coded stack, heap, data, memory
  - other debuggers (python, rust, node, ...)
  - bash
  - top
  - vim
  - plugin creator layout
  - fancy notcurses app (?)


- IMPLEMENTATION - gdb

  - windows
    - scrolling
    - focus changes border color

    - nested window actions

      - (b)reakpoints
        - b -> focus window
          - arrow keys -> scrolling
          - / + line number + Enter  -> add breakpoint
            - ctrl + >/< -> jump to next/prev delimiter or type in delimiter 
              - ' ', ')', ']', ...

      - (w)atches
        - w -> focus
        - arrow keys -> scrolling
        - / + variable + Enter  -> add watch
          - while typing, matches highlighted in source window
          - tab -> skip to next match, change word in front of '/'

  - source code regularly checks for source code changes
    - window notification

  - normal or reverse mode toggle

  - persistent custom variable definitions in .gdbinit, .termIDE, ...?

  - WINDOW - break, catchpoints
    - can set in source window
    - labeled with b1, c2

  - WINDOW - variables
    - numbered for easy selection
    - no wrapping but truncated so that single value visible
      - select + `Enter` -> 
        - see: "selected item output" above
    - j/k scrolling
    - local + watched variables
      - sorted together by default
      - can shift values up/down
    - highlight changed values
    - select + `Enter`

  - WINDOW - single array, formatted
    - handles static and dynamic
    - persistent array selections in .termIDE

  - WINDOW - source code
    - window popup that lists all source files to choose from
    - add break, watch, catchpoint at current line
    - points labeled with id number  
      - b1, w2, c3
      - replace line number to save space (?)
    - add syntax highlighting

  - run macro functions

      [layout:gdb1]
      ...
      >gdb_macros
      g:my_command i 10

  - definition popup or other window 
    - functions and variables under cursor
    - edit selection by pressing '/'
    - autocomplete with tab
    - maintains current highlight in source window






- IMPLEMENTATION - bash

  - mkfifo option
    - NOT for this, use pty
    - useful for other things potentially

    - input terminal

      ```
      $ mkfifo mypipe
      ```

    - output terminal

      ```
      #!/bin/bash

      while true; do
          cat mypipe
      done
      ```

    - input terminal

      ```
      $ printf "\e[34m$(basename "$(pwd)")/ $ \e[0mll\n" > mypipe 2>&1; ll > mypipe 2>&1
      ```




