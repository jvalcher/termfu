
# termfu

A fast, multi-language TUI debugger that allows users to easily create and switch between custom layouts
<br><br>

<img src='./misc/layout1.png' height='400px'>


### Support

| `Debugger` | `Languages` |
|   :----:   | :----: |
| `gdb`      | C, C++, D, Go, Objective-C, <br>Fortran, OpenCL C, Pascal, <br>Rust, assembly, Modula-2, Ada |
| `pdb`      | Python |
<br>


## Usage

### Dependencies

```
sudo apt-get install libncurses-dev gdb
```
<br>

The Makefile uses `ncurses6`.
<br><br>

### Installation
```
make
```
<br>

### Run test files

Comment out the undesired command in the provided `.termvu` configuration file.

GDB
<br>
```
(cd misc && ./build_hello) && ./termfu
```
<br>

PDB
<br>
```
./termfu
```
<br><br>


## Configuration

Each unique, three-character, case-sensitive plugin code corresponds to a specific debugger action or window. These codes are mapped to user-defined keys, which are used to create custom ASCII-art layouts that users can switch between.
<br>

Run the program in the same directory as the configuration file `.termfu`. Breakpoints and watchpoints are persisted in `.termfu_data`.
<br><br>


### Plugins

Header Commands
<br>
| Code    | Description |
| :-----: | ------ |
| Con     | Continue |
| Fin     | Finish |
| Kil     | Kill |
| Lay     | Choose layout |
| Nxt     | Next |
| Prm     | Debugger prompt |
| Qut     | Quit termvu |
| Run     | Run program |
| Stp     | Step |
| Unt     | Until |
<br>

Windows
<br>
| Code    | Description        | GDB                | PDB                |
| :-----: | -----              | :-----:            | :------:           |
| Asm     | Assembly code      | :heavy_check_mark: |                    |   
| Brk     | Breakpoints        | :heavy_check_mark: | :heavy_check_mark: |
| Dbg     | Debugger output    | :heavy_check_mark: | :heavy_check_mark: |
| LcV     | Local variables    | :heavy_check_mark: |                    |
| Prg     | Program CLI output | :heavy_check_mark: |                    |
| Reg     | Registers          | :heavy_check_mark: |                    |
| Src     | Source file        | :heavy_check_mark: | :heavy_check_mark: |
| Wat     | Watchpoints        | :heavy_check_mark: | :heavy_check_mark: |
<br>


### Configuration sections

Adding parentheses around a character in the `<(t)itle>` changes the character's color.
<br>

| Section   | Description |
| :----:    |  :----: |
| `command` | Debugger command |
| `plugins` | `<plugin code>` : `<key binding>` : `<title>` |
| `layout`  | `[ layout : <title> ]` <br>`>h` : header commands,  `>w` : windows |
<br>


### Example `.termfu` configuration

Only newline comments are supported, not inline.
<br>

```
[ command ]

gdb --quiet --interpreter=mi misc/hello

[ plugins ]

# windows
Asm : a : (a)ssembly
Brk : e : br(e)akpoints
Dbg : d : (d)ebug out
LcV : v : local (v)ars
Prg : p : (p)rogram out
Reg : g : re(g)isters
Src : o : s(o)urce file
Wat : w : (w)atch

# header commands
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

# header commands layout
>h
mlq
rnscufk

# ASCII window layout
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

Window data is scrollable using arrow or `hjkl` keys.
<br>

<img src='./misc/layout1.png' height='400px'>
<img src='./misc/layout2.png' height='400px'>
<br>


## Contributing

Minor fixes are welcome as a pull request. Please create an issue before embarking on any significant fixes, changes, or additions. Run `make todo` to view `TODO`, `FIX`, etc. tags in the source code.
<br>


### Scripts

| Command                     | Description |
| --------                    | -------     |
| `make`                      | Build production binary |
| `make allf`                 | Build production binary, print formatted error messages |
| `make dev`	                | Build development binary, run it |
| `make devf`                 | Build development binary, print formatted error messages |
| `make test T=tests/test1.c` | Run `tests/test1.c` |
| `make debug`                | Run Tmux debugging session |
| `make todo`                 | Print source code tags  (`TODO`, `FIXME`, etc.) |
| `make colors`	              | Check if current terminal can display colors with Ncurses |

<br><br>

