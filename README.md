
<h1 align="center">termfu</h1>

A multi-language debugger frontend for the Linux terminal

- ___Fast___
- Easy to configure
- Single-key bindings documented on-screen
- Switch between custom layouts
- Scroll through window data
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

## Install from source

### Dependencies

Ubuntu:
```
sudo apt-get install make gcc libncurses-dev gdb python3
 ```

Fedora:
```
sudo dnf install make gcc ncurses-devel gdb python3
```

### Build
```
make
sudo make install
```
<br>


## Usage

- The program can be run as follows in the same directory as a `.termfu` configuration file.
- Breakpoint and watchpoint data is stored in `.termfu_data`.
```bash
termfu
```

- `-h` prints usage instructions.
- `-c` sets the configuration file path.
- `-p` sets the data persistence file path.
```bash
termfu -c .termfu_01 -p .termfu_01_data
```

- Configure the debugger command, key bindings, titles, and layouts inside the configuration file  _(see below)_.
- Switch between layouts with your `(l)ayouts` key binding.
- A window's data can be scrolled through by selecting it with its key binding and then using the arrow or `hjkl` keys.
<br><br>


## Configuration

### Example `.termfu` configuration


```
[ command ]

gdb --quiet --interpreter=mi misc/vars

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
Brk : e : br(e)akpoints
Dbg : d : (d)ebug out
LcV : v : local (v)ars
Prg : p : (p)rogram out
Reg : g : re(g)isters
Src : o : s(o)urce file
Stk : T : s(T)ack
Wat : w : (w)atch


[ layout : Main ]

>h
mlqrns
cufk

>w
eeeooooo
wwwooooo
vvvooooo
TTpppddd

[ layout : Assembly / Registers ]

>h
mlqrns
cufk

>w
oag
oag
oag
wdT
```
<br>


### Resulting layouts

<img src='./misc/layout1.png'>
<img src='./misc/layout2.png'>
<br>

### Sections
- Only newline `# comments` are supported, not inline.

| Section   | Description |
| :----:    |  :----: |
| command | Debugger command |
| plugins | \<plugin code\> : \<key binding\> : \<title\> |
| layout  | \[ layout : \<label\> \] <br>`>h` : header commands,  `>w` : windows |
<br>

### Commands

```
[ command ]

gdb --quiet --interpreter=mi misc/vars

````

| Debugger | Command |
| :-----:  | ------  |
| GDB      | `gdb --interpreter=mi ...` |
| PDB      | `python -m pdb ...` |
<br>

### Plugins

```
[ plugins ]

# <plugin code> : <key binding> : <title>
AtP : A : (A)ttach
Con : c : (c)ontinue

```

- Each three-character, case-sensitive plugin code corresponds to a specific header command or window. 
- Add parentheses around key binding in its `<(t)itle>` for easy reference. This character will have a different color than the surrounding characters.

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

- The order and row of each header command title is set by the order and row of its key binding.
- Window size ratios and position are set via "key-binding ASCII art."
<br>


## Debugging tips

### Easy Vim, Neovim breakpoints

Use these functions to create and copy a breakpoint string (`<file>:<line>`) from the current line to paste into `termfu`'s breakpoint window.

__Vim__
```vim
  function! CreateBreakpoint()
      let l:filename   = expand('%:t')
      let l:linenumber = line('.')
      let l:breakpoint = l:filename . ':' . l:linenumber
      let @+           = l:breakpoint
      echo l:breakpoint
  endfunction
  nnoremap <leader>b :call CreateBreakpoint()<CR>
```

__Neovim__
```lua
local function create_break ()
    local filename   = vim.fn.expand('%:t')
    local linenumber = vim.fn.line('.')
    local breakpoint = filename .. ':' .. linenumber
    vim.fn.setreg('+', breakpoint)
    print(breakpoint)
end
vim.keymap.set('n', '<leader>b', create_break, {desc = 'Create debugger breakpoint string'});
```
<br>

## Contributing

### General Guidelines
- For any significant contributions outside of patches, open an issue first.
- Bug fixes, optimizations, new debugger implementations, and plugins are welcome.
- Use existing code conventions.

### Developer notes
- Run `make help` to print all scripts.
- Run `make todo` to print all source file tags, such as `TODO`, `FIX`, etc.
- Run `make build` to compile all test programs.
- You can debug (and explore) `termfu_dev` with `termfu`.
  - Run `$ make debug`, which requires `termfu` to be in your path.
  - Open a separate terminal or pane for the debugged process
  - Run `$ tty` and copy the terminal file name's path.
  - Run `$ sleep 99999`.
  - Back in your debugger, run `>>> tty /dev/pts/<x>` at the prompt.
  - Start debugging.
  - You can kill the debugged process with `>>> signal 2`.
- The `logd()` function in `src/utilities.h` allows for `printf()`-style debugging when running `ncurses` by outputting to `debug.out`.
- Run `make help` in the `tests/` directory to view available scripts.
- All errors that require the program to exit must propagate back to `main()` using the formatted error macro functions in `utilities.h`. Standard library function error checking must include the `errno` message when available. Refer to the source code for examples of this.
- It is recommended to create a shortcut for refreshing your terminal screen, as `ncurses` will make a mess of it when not shut down properly.

<br><br>

