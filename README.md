
# termvu

An easy-to-configure, multi-language terminal debugger
<br /><br />
*** __In development__ ***
<br /><br />
<img src='./misc/layout1.png' height='400px'>
<br /><br />

## Supported debuggers
| `gdb`  | `pdb`... |
| :----: | :----: |

<br /><br />

## Configuration

Unique, three-character, case-sensitive "plugin" codes are each associated with a specific debugger action or window. The codes are bound to user-defined shortcut keys. The keys are then used to create custom ASCII-art layouts.
<br />

### Example configuration

```
[ plugins ]

Asm : a : (a)ssembly
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


[ layout : Main ]

>h
lq
rnscfk

>w
eooaa
wooaa
vppdd


[ layout : Assembly / Registers ]

>h
lq
rnscfk

>w
oag
```


### Resulting layouts

<img src='./misc/layout1.png' height='500px'>
<img src='./misc/layout2.png' height='500px'>
<br />


### Configuration file

Run the program from the same directory as the configuration file.
<br />
`./.termvu` 
<br /><br />

## Scripts
<br />

| Command                | Description |
|------------------------|-------------|
| `make`                 | Build production binary |
| `make allf`            | Build production binary, print formatted error messages |
| `make dev`	            | Build development binary, run it |
| `make devf`            | Build development binary, print formatted error messages |
| `make test T=<path>`   | Run `tests/test1.c` |
| `make todo`            | Print source code tags  (`TODO`, `FIXME`, etc.) |
| `make colors`	         | Check if current terminal can display colors |

<br /><br />

