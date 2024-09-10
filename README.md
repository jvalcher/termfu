
# termvu

A fast, multi-language TUI debugger that allows users to easily create and switch between custom layouts
<br><br>

<img src='./misc/layout1.png' height='400px'>


### Support

| `Debugger` | `Languages` |
|   :----:   | :----: |
| `gdb`      | C, C++, D, Go, Objective-C, <br>Fortran, OpenCL C, Pascal, <br>Rust, assembly, Modula-2, Ada |
| `pdb`      | Python <br>(_in development_) |
<br>


## Usage

### Dependencies

`$ sudo apt-get install python3 libncurses-dev gdb`
<br>

The Makefile uses ncurses6.
<br>

### Installation
`$ make`
<br>

### Run
`$ termvu -d gdb a.out`
<br><br>


## Configuration

Each unique, three-character, case-sensitive plugin code corresponds to a specific debugger action or window. These codes are mapped to user-defined keys, which are used to create custom ASCII-art layouts that users can switch between.
<br>

Run the program in the same directory as the configuration file `.termvu`. Breakpoints and watchpoints are persisted in `.termvu_data`.
<br><br>


### Plugins

| Code  | Type  | Description |
| ----- | ----- | ----- |
| Asm | Window | Assembly code |
| Brk | Window | Breakpoints |
| Con | Header | Continue |
| Dbg | Window | Debugger output |
| Fin | Header | Finish |
| Kil | Header | Kill |
| Lay | Header | Choose layout |
| LcV | Window | Local variables |
| Nxt | Header | Next |
| Prg | Window | Program CLI output |
| Prm | Header | Debugger prompt |
| Qut | Header | Quit termvu |
| Reg | Window | Registers |
| Run | Header | Run program |
| Src | Window | Source file |
| Stp | Header | Step |
| Unt | Header | Until |
| Wat | Window | Watchpoints |
<br>


### Configuration sections

Adding parentheses around a character in the `<(t)itle>` changes the character's color.
<br>

| Section   | Description |
| :----:    |  :----: |
| `plugins` | `<plugin code>` : `<key binding>` : `<title>` |
| `layout`  | `[ layout : <title> ]` <br>`>h` : header commands,  `>w` : windows |
<br>


### Example `.termvu` configuration

```
[ plugins ]

Asm : a : (a)ssembly
Brk : e : br(e)akpoints
Dbg : d : (d)ebug out
LcV : v : local (v)ars
Prg : p : (p)rogram out
Reg : g : re(g)isters
Src : o : s(o)urce file
Wat : w : (w)atch

Prm : m : pro(m)pt
Lay : l : (l)ayouts
Qut : q : (q)uit
Run : r : (r)un
Nxt : n : (n)ext
Stp : s : (s)tep
Con : c : (c)ontinue
Unt : u : (u)ntil
Fin : f : (f)inish
Kil : k : (k)ill

[ layout : Main ]

>h
mlq
rnscufk

>w
eooaa
wooaa
vppdd

[ layout : Assembly / Registers ]

>h
mlq
rnscufk

>w
oag
```
<br>


### Resulting layouts

All window data is scrollable using arrow keys or `hjkl`.
<br>

<img src='./misc/layout1.png' height='400px'>
<img src='./misc/layout2.png' height='400px'>
<br>


## Scripts

| Command                | Description |
|------------------------|-------------|
| `make`                 | Build production binary |
| `make allf`            | Build production binary, print formatted error messages |
| `make dev`	           | Build development binary, run it |
| `make devf`            | Build development binary, print formatted error messages |
| `make test T=<path>`   | Run `tests/test1.c` |
| `make debug`           | Run Tmux debugging session |
| `make todo`            | Print source code tags  (`TODO`, `FIXME`, etc.) |
| `make colors`	         | Check if current terminal can display colors with Ncurses |

<br><br>

