
# termIDE

A multi-language terminal debugger for GNU/Linux

**\*\* In development \*\***


## To do

### Bookmark == "CURRENT"

-> Last task: Read output buffer to file, hook up scrolling, ...

- [x] Render layout from configuration file

- [x] Attach keyboard shortcuts to plugin functions

- [x] Add title string color pulse, switch to indicate usage

- [x] Clean out utilities.c

- [ ] Add debugger, plugins functionality
  - [x] Add plugin function interface
  - [x] Add select window coloring
  - [ ] Add DEBUG macros and functionality (no Ncurses, only data)
  - [ ] Parse debugger output to files
    - [ ] debugger
    - [ ] program
    - [ ] window data
  - [ ] Update window data
  - [ ] Add window functionality
    - [ ] Scroll
    - [ ] Debugger-specific
  - [ ] Add popup windows
  - [ ] Make window data persistent
  - [ ] Format window data
    - [ ] source file -> source-highlight
  - [ ] Combine commits, merge

- [ ] Add help menus
  - [ ] Main
  - [ ] Per window

- [ ] Add switch-layout functionality

- [ ] Figure out, cull refresh(), wrefresh() calls

- [ ] Add cli flags

- [ ] Incorporate errno messages into pfem

- [ ] Add other debuggers
  - [ ] pdb
  - [ ] node --inspect

- [ ] Update, fix comments

- [ ] Add PLUGINS.md

- [ ] Research open source repo management
  - [ ] Add CONTRIBUTING.md
  - [ ] Add CHANGES.md
  - [ ] Add LICENSE.md
  - [ ] Update, fix README.md

- [ ] Add issues
  - [ ] Convert processes to threads where possible
  - [ ] Convert corner fixes to bitwise operations

- [ ] Make repo public





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

