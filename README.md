
# Terminal Debugger

A multi-language TUI debugger

**\*\* In development \*\***


## Configuration

Unique, three-character, case-sensitive "plugin" codes are each associated with a specific debugger action or window. The codes are bound to user-defined shortcut keys. The keys are then used to create custom ASCII-art layouts.
<br />

### Example configuration...

```
[ plugins ]

Asm : a : (a)ssembly
Bak : b : (b)ack
Bld : u : b(u)ilds
Brk : e : br(e)akpoints
Con : c : (c)ontinue
Dbg : d : (d)ebug out
Fin : f : (f)inish
Kil : k : (k)ill
Lay : l : (l)ayouts
LcV : v : local (v)ars
Nxt : n : (n)ext
Prg : p : (p)rogram out
Qut : q : (q)uit
Reg : g : re(g)isters
Run : r : (r)un
Src : o : s(o)urce file
Stp : s : (s)tep
Wat : w : (w)atch


[ layout : gdb_debug_1 ]

>h
ulbq
rnscfk
>w
eooaa 
wooaa
vooaa
gppdd

```


### Resulting layout...

<img src='./misc/layout.png' height='500px'>
<br />


### File location

`$PROJECT_DIR/.term_debug` <br />
or <br />
`$HOME/.term_debug`
<br /><br />
 

### Formatting

#### Plugins ( `[ plugins ]` ):

- `<code> : <key> : <title>`

  - `code`
    - Plugin code identifier

  - `key`
    - Key binding

  - `title`
    - String displayed in header or window
    - Adding parentheses around the key binding allows term_debug to change its color for better visibility.
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
<br /><br />


## Build

### Production
```
$ make
```
### Development
```
$ make dev
```
<br /><br />

