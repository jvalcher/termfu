
# termIDE

An ncurses debugger frontend for the Linux terminal

**\*\* In development \*\***

## Configuration file overview

Unique, three-character, case-sensitive plugin codes are bound to specific debugger actions. These codes are bound to shortcut keys, which can then be used to create a layout.

#### Formatting

File name:

```
./.termide
```

Layout header title format:

  `[[ <type> : <title string> ]]`

Sections:

```
>p  plugin code, key binding, title string
>h  determines order and layout of plugin title strings in header
>w  window layout
    - each keybinding (u, l, q, etc.) corresponds to a "window segment"
    - the size of each window is determined by how many segments it has for its
      width and height
    - segment sizes are calculated based on the current terminal's dimensions
```

Plugin (`>p`) formatting:

  `<code> : <key> : <window/action title string>`

#### Example configuration file...

```
[ shortcuts ]

Bld : u :b(u)ilds
Lay : l :(l)ayouts
Bak : q :(q)uit
Run : r :(r)un
Nxt : n :(n)ext
Stp : s :(s)tep
Con : c :(c)ontinue
Fin : i :f(i)nish
Kil : k :(k)ill
Brk : b :(b)reakpoints
Src : f :source (f)ile
Asm : a :(a)ssembly
Reg : e :r(e)gisters
Prm : p :(p)rompt
Wat : w :(w)atch
LcV : v :local (v)ars
Out : o :(o)utput

[ layout : gdb_debug_1 ]

>h
ulq
rnscfk
>w
bffaa 
wffaa
lffaa
eoopp
```

#### Resulting layout...

<img src='./misc/screenshots/layout.png' height='500px'>


## Build

Production
```
$ make
```
Development
```
$ make dev
```

## To do

- [x] Render layout from configuration file
- [x] Attach keyboard shortcuts to plugin functions
- [x] Add title string color pulse, switch to indicate usage
- [ ] Add debugger, plugins functionality
  - Plugin commands
    - Change "plugins" to "shortctus" in config file
    - Make commands generic between debuggers
  - Read output to files
  - Parse output
  - Update windows
- [ ] Add window functionality
  - Scroll
  - Debugger-specific
- [ ] Add help menus
  - Main
  - Window
- [ ] Add switch-layout functionality
- [ ] Add other debuggers
  - pdb
  - node inspect
- [ ] Update README.md
- [ ] Add DEVELOP.md
  - Code changes must be reflected here

