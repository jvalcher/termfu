
<h1 align="center">termfu</h1>

A multi-language debugger frontend for the Linux terminal

- ___Fast___
- Easy to configure
- Key bindings documented on-screen
- Switch between custom layouts while debugging
- Scrollable window data
- Breakpoint, watchpoint persistence
- Debugger prompt access

<br>
<p align="center"><img margin-left="auto" src="./misc/layout1.png"></p>
<br>


## Support

| Debugger   | Languages |
|   :----:   | -----     |
| [GDB](https://sourceware.org/gdb/) | C, C++, D, Go, Objective-C, Fortran, OpenCL C, Pascal, Rust, assembly, Modula-2, Ada |
| [PDB](https://docs.python.org/3/library/pdb.html) | Python |
<br>

## Installation

### Dependencies

```
sudo apt-get install make gcc libncurses-dev gdb python3
 ```

### Build and install
```
make
sudo make install
```
<br>


## Usage

- The program can be run as follows in the same directory as a `.termfu` configuration file.
- Breakpoint and watchpoint data will be stored in `.termfu_data`.
```bash
termfu
```

### Flags
- `-h` prints usage instructions.
- `-c` sets the configuration file path.
- `-p` sets the data persistence file path.
```bash
termfu -c .termfu_01 -p .termfu_01_data
```

### Basics
- Switch between layouts with your `(l)ayouts` key.
- Scroll window data by selecting the window via its key binding and then using the arrow, page up, etc. keys. One can also use the `hjkl` keys.
<br><br>

## Configuration

### Example `.termfu` configuration


```

[ command ]

gdb --interpreter=mi test_programs/hello

[ plugins ]

# header commands
AtP : A : (A)ttach
Con : c : (c)ontinue
Fin : f : (f)inish
Kil : k : (k)ill
Lay : l : (l)ayouts
Nxt : n : (n)ext
Prm : m : pro(m)pt
Qut : q : (q)uit
Run : r : (r)un
Stp : s : (s)tep
Unt : u : (u)ntil

# windows
Asm : a : (a)ssembly
Brk : b : (b)reakpoints
Dbg : d : (d)ebug out
LcV : v : local (v)ars
Prg : p : (p)rogram out
Reg : g : re(g)isters
Src : o : s(o)urce file
Stk : t : s(t)ack
Wat : w : (w)atch


[ layout : Main ]

>h
mlq
rnscufk

>w
bbbooooo
wwwooooo
vvvooooo
ttpppddd

[ layout : Assembly / Registers ]

>h
mlq
rnscufk

>w
oag
oag
oag
wdt

```
<br>


### Resulting layouts

<img src='./misc/layout1.png'>
<img src='./misc/layout2.png'>
<br>

### Command

```
[ command ]

gdb --interpreter=mi misc/vars

````

| Debugger | Command |
| :-----:  | ------  |
| GDB      | `gdb --interpreter=mi a.out` |
| PDB      | `python -m pdb app.py` |
<br>

### Plugins

```
[ plugins ]

# <plugin code> : <key binding> : <title>

AtP : A : (A)ttach
Con : c : (c)ontinue

```

- Each three-character, case-sensitive plugin code corresponds to a specific header command or window. 
- Add parentheses around a key binding in its `(t)itle` for easy reference. This character will have a different color than the surrounding characters.

<br>

__Header Commands__
<br>

| Code    | Description           | GDB                | PDB                |
| :-----: | ------                | :----:             | :----:             |
| AtP     | Attach to PID, file   | :heavy_check_mark: |                    |
| Con     | Continue              | :heavy_check_mark: | :heavy_check_mark: |
| Fin     | Finish                | :heavy_check_mark: | :heavy_check_mark: |
| Kil     | Kill                  | :heavy_check_mark: | :heavy_check_mark: |
| Lay     | Choose layout         | :heavy_check_mark: | :heavy_check_mark: |
| Nxt     | Next                  | :heavy_check_mark: | :heavy_check_mark: |
| Prm     | Debugger prompt       | :heavy_check_mark: | :heavy_check_mark: |
| Qut     | Quit                  | :heavy_check_mark: | :heavy_check_mark: |
| Run     | Run, reload program   | :heavy_check_mark: | :heavy_check_mark: |
| Stp     | Step                  | :heavy_check_mark: | :heavy_check_mark: |
| Unt     | Until                 | :heavy_check_mark: | :heavy_check_mark: |
<br>

__Windows__
<br>

| Code    | Description      | GDB                | PDB                |
| :-----: | -----            | :-----:            | :------:           |
| Asm     | Assembly code    | :heavy_check_mark: |                    |   
| Brk     | Breakpoints      | :heavy_check_mark: | :heavy_check_mark: |
| Dbg     | Debugger output  | :heavy_check_mark: | :heavy_check_mark: |
| LcV     | Local variables  | :heavy_check_mark: | :heavy_check_mark: |
| Prg     | Program output   | :heavy_check_mark: | :heavy_check_mark: |
| Reg     | Registers        | :heavy_check_mark: |                    |
| Src     | Source file      | :heavy_check_mark: | :heavy_check_mark: |
| Stk     | Stack            | :heavy_check_mark: | :heavy_check_mark: |
| Wat     | Watchpoints      | :heavy_check_mark: | :heavy_check_mark: |
<br>


### Layouts

```
[ layout : Main ]

>h
mlqrns
cufk

>w
eeeooooo
wwwooooo
vvvooooo
TTpppddd
```

- The order and row of each header (`>h`) key binding in the configuration determines the corresponding title's position in the layout.
- Window (`>w`) size ratios and positions are determinded via "key-binding ASCII art."
<br>


## Contributing

### General Guidelines
- Bug fixes, optimizations, new debugger implementations, and plugins are welcome.
- Use existing code conventions.

### Developer notes
- Run `make help` to print all scripts.
- Run `make todo` to print all source file tags, such as `TODO`, `FIX`, etc.
- Run `make build` to compile all test programs.
- The `logd()` function in `src/utilities.h` allows for `printf()`-style debugging when running `ncurses` by outputting to `debug.out`.
- It is recommended to create a shortcut for refreshing your terminal screen, as `ncurses` will make a mess of it when not shut down properly.

### Debugging `termfu_dev` with `termfu`
- Run `$ make debug` to start the termfu debugger. This requires `termfu` to be in your path.
- Open a separate terminal for the debugged process.
- Run `$ tty` and copy the terminal file name's path, e.g. `/dev/pts/1`.
- Run `$ sleep 99999`.
- Back in the debugger, open the prompt and run `>>> tty /dev/pts/<x>`, substituting your debugged process's terminal file name.
- Set your breakpoint and start debugging.
- Adjust `configs/.termfu_debugger` and `configs/.termfu_debugged` as needed

<br><br>
