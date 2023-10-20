
# termIDE

A stress-free TUI development environment

- Fast performance
- Fast navigation
- Simple configuration
- Multiple custom window layouts
- Built-in plugin creator
- Documented within an inch of its life


# Uses

- Debugger



## To do

- config file name can be ".termide*" 

- basic plugin functionality

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

- plugin creator layout

- gdb plugins

  - windows
    - scrolling
    - focus changes border color

  - breakpoints
    - can set in source window

  - source code
    - window popup that lists all source files to choose from
    - syntax highlighting  (onedark?)

  - normal or reverse mode toggle

- software builds

  - list bash commands under each [build_01] config heading
  - runs first build in config file by default

- bash
  - popen() ?

- vim

- top

- Tetris
  - Notcurses

