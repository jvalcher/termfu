
# termIDE

A multi-language terminal debugger for GNU/Linux

**\*\* In development \*\***


## To do

-> Last task: Read output buffer to file, hook up scrolling, ...

- [x] Render layout from configuration file

- [x] Attach keyboard shortcuts to plugin functions

- [x] Add title string color pulse, switch to indicate usage

- [ ] Add debugger, plugins functionality
  - [*] Make commands generic between debuggers (interface)
  - [ ] Add window updating to reader process
    - create win_update_t linked list  { WINDOW, code, path }
    - receives signal from reader process to get code in win_update.code
    - matches code in linked list
    - updates WINDOW from data in path
  - [ ] Read output to files
  - [ ] Parse output
  - [ ] Update windows
  - [ ] Make window data persistent
  - [ ] Look into better error handling (errno, ...)
  - [ ] Clean out utilities.c
  - [ ] Combine commits

- [ ] Add DEBUG macros and functionality (no Ncurses, only data)

- [ ] Add window functionality
  - [ ] Scroll
  - [ ] Debugger-specific

- [ ] Add help menus
  - [ ] Main
  - [ ] Per window

- [ ] Add switch-layout functionality

- [ ] Add other debuggers
  - [ ] pdb
  - [ ] node --inspect

- [ ] Update, fix comments

- [ ] Update, fix README.md

- [ ] Add PLUGINS.md

- [ ] Add CONTRIBUTING.md

- [ ] Add CHANGES.md

- [ ] Add LICENSE.md




## Configuration

### The idea

Unique, three-character, case-sensitive "plugin" codes are each associated with a specific debugger action or window. The codes are bound to user-defined shortcut keys. The key characters are used to create custom layouts.
<br />

### Example configuration...

```
[ plugins ]

Qut : q : (q)uit
Bak : b : (p)rev
Bld : u : b(u)ilds
Lay : l : (l)ayouts
Run : r : (r)un
Nxt : n : (n)ext
Stp : s : (s)tep
Con : c : (c)ontinue
Fin : i : f(i)nish
Kil : k : (k)ill
Brk : b : (b)reakpoints
Src : f : source (f)ile
Asm : a : (a)ssembly
Reg : e : r(e)gisters
Prm : m : pro(m)pt
Wat : w : (w)atch
LcV : v : local (v)ars
Out : o : (o)utput


[ layout : gdb_debug_1 ]

>h
ulpq
rnscfk

>w
bffaa 
wffaa
lffaa
eoopp
```


### Resulting layout...

<img src='./misc/screenshots/layout.png' height='500px'>
<br /><br /><br />


### File location

`$PROJECT_DIR/.termide` <br />
or <br />
`$HOME/.termide`
<br /><br />
 

### Formatting
<br />

#### Plugins ( `[ plugins ]` ):

- `<code> : <key> : <title>`

  - `code`
    - Plugin code identifier

  - `key`
    - Key binding

  - `title`
    - String displayed in header or window
    - Adding parentheses around the key binding allows termIDE to change its color for better visibility.
<br />

#### Layout ( `[ layout : gdb_debug_1 ]` ):

- Title ( `gdb_debug_1` ):
  - Title displayed in header
  - Identifier used by layouts plugin (`Lay`) to switch layouts
<br /><br />

- Window sections:
  - The characters under each `>section` correspond to the plugin key bindings defined under `[ plugins ]`. 

  - `>h`  
    - Header window 
      - Each character determines the row and position of its corresponding title string.

  - `>w`  
    - Windows layout
      - Each character represents a "segment" of that particular window.
      - The number of segments a window has vertically and horizontally determines the number of terminal rows and columns it is assigned.
<br />

- **NOTE**:  Adding a non-window plugin to the header or vice-versa will result in an error (see PLUGINS.md).
<br /><br />


## Plugins

See `PLUGINS.md` for a list of available plugins.
<br /><br />


## Debuggers

- See `CONTRIBUTING.md` for information about adding functionality to the following debuggers and adding new debuggers.

- GDB 
  - C/C++
  - D
  - Go
  - Objective-C
  - OpenCL C
  - Fortran
  - Pascal
  - Rust
  - Modula-2
  - Ada

- PDB
  - Python

- node --inspect
  - NodeJS


## Builds

### Production
```
$ make
```
### Development
```
$ make dev
```
<br /><br />

