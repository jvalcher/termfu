
<h1 align="center">termfu</h1>
<p align="center">An easy-to-use, multi-language terminal debugger that allows users to create and switch between custom layouts</p>
<br>
<p align="center"><img margin-left="auto" src="./misc/layout1.png" width="700px"></p>
<br>


## Support

| Debugger   | Languages |
|   :----:   | -----     |
| [GDB](https://sourceware.org/gdb/) | C, C++, D, Go, Objective-C, Fortran, OpenCL C, Pascal, Rust, assembly, Modula-2, Ada |
| [PDB](https://docs.python.org/3/library/pdb.html) | Python |
<br>

## Install from source

### Dependencies

The Makefile uses `ncurses6`.
```
sudo apt-get install libncurses-dev gdb pdb make gcc
```
### Build
```
make
```
<br>

Copy `termfu` to the desired executable directory, e.g. `/usr/bin`. 
<br><br>


## Usage

- The program can be run as follows in the same directory as a `.termfu` configuration file.
```
termfu
```
- Set the configuration file with `-c`. Breakpoints and watchpoints will be persisted in `.termfu_data` unless specified with the `-d` flag.
```
termfu -c configs/.termfu_01 -d data/.termfu_01_data
```
- Configure the debugger command, key bindings, titles, and layouts inside the configuration file  (see below).
- Window data can be scrolled through using the arrow or `hjkl` keys.
<br><br>


## Configuration
Each three-character, case-sensitive plugin code corresponds to a specific header command or window. 
<br>

### Plugins

__Header Commands__
<br>

| Code    | Description |
| :-----: | ------ |
| Con     | Continue |
| Fin     | Finish |
| Kil     | Kill |
| Lay     | Choose layout |
| Nxt     | Next |
| Prm     | Debugger prompt |
| Qut     | Quit |
| Run     | Run program (reload binary if needed) |
| Stp     | Step |
| Trg     | Target remote server (e.g. gdbserver) |
| Unt     | Until |
<br>

__Windows__
<br>

| Code    | Description      | GDB                | PDB                |
| :-----: | -----            | :-----:            | :------:           |
| Asm     | Assembly code    | :heavy_check_mark: |                    |   
| Brk     | Breakpoints      | :heavy_check_mark: | :heavy_check_mark: |
| Dbg     | Debugger output  | :heavy_check_mark: | :heavy_check_mark: |
| LcV     | Local variables  | :heavy_check_mark: |                    |
| Prg     | Program output   | :heavy_check_mark: |                    |
| Reg     | Registers        | :heavy_check_mark: |                    |
| Src     | Source file      | :heavy_check_mark: | :heavy_check_mark: |
| Stk     | Stack            | :heavy_check_mark: |                    |
| Wat     | Watchpoints      | :heavy_check_mark: | :heavy_check_mark: |
<br>


### Configuration setup

Plugin codes are mapped to case-sensitive letter keys, which are used both as shortcuts and to create custom layouts.
<br>

- Add parentheses around a shortcut key in its `<(t)itle>` for easy reference. These characters will have a different color than the surrounding letters.
- Only newline `# comments` are supported, not inline.

### Sections

| Section   | Description |
| :----:    |  :----: |
| command | Debugger command |
| plugins | \<plugin code\> : \<key binding\> : \<title\> |
| layout  | \[ layout : \<label\> \] <br>`>h` : header commands,  `>w` : windows |

### Supported commands

| Debugger | Command |
| :-----:  | ------  |
| GDB      | `gdb --interpreter=mi ...` |
| PDB      | `python -m pdb ...` |
<br>

### Example `.termfu` configuration


```
[ command ]

gdb --interpreter=mi misc/hello

[ plugins ]

# header commands
Prm : m : pro(m)pt
Trg : t : (t)arget
Lay : l : (l)ayouts
Qut : q : (q)uit
Run : r : (r)un
Nxt : n : (n)ext
Stp : s : (s)tep
Con : c : (c)ontinue
Unt : u : (u)ntil
Fin : f : (f)inish
Kil : k : (k)ill

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
mtlq
rnscufk

>w
eeeooooo
wwwooooo
vvvooooo
TTpppddd

[ layout : Assembly / Registers ]

>h
mtlq
rnscufk

>w
oag
oag
oag
wpT
```
<br>


### Resulting layouts

<img src='./misc/layout1.png' height='400px'>
<img src='./misc/layout2.png' height='400px'>
<br>

## Debugging tips

### Easy Vim, Neovim breakpoints

Use these functions to create and copy a breakpoint string (`<file>:<line>`) from the current line to paste into `termfu`'s breakpoints window.

__Vim__
```vimscript
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
    local filename = vim.fn.expand('%:t')
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
- For any significant contributions outside of minor patches, open an issue first.
- Bug fixes, optimizations, new debugger implementations, and new plugins are welcome.
- Blank space and comment PRs have a high probability of being closed.
- Use existing code conventions.

### Developer notes
- Run the `make configs` script to create all needed configuration files in `scripts/` for running the included sample binaries or scripts (`make run_dev_gdb`, etc.) and for debugging. Feel free to edit the `scripts/create_configs` script to customize layouts, change the target binary, add plugins, etc. However, do __NOT__ include your customized script in a PR.
- All watchpoints and breakpoints will be persisted in `_data` files alongside their relevant configuration files in `scripts/`.
- Run `make todo` to print all source file tags, such as `TODO`, `FIX`, etc.
- The `make tui_` scripts start a `tmux`-based `GDB` TUI debugging session.
- The `make server` and `make target` scripts allow `termfu` to debug itself. __Note__: stepping through the program using these scripts is currently slow for some reason. There is an `OPTIMIZE` source tag to speed this up.
- The `logd()` function in `src/utilities.h` allows for `printf()`-style debugging when running `ncurses` by outputting to `debug.out`.
- It is recommended to create a shortcut for refreshing your terminal screen, as `ncurses` will make a mess of it when not shut down properly. For example, add `bind r respawn-pane -k` to `~/.tmux.conf` to refresh your `tmux` pane with `Ctrl+b` then `r`.


### Scripts

`./Makefile`
<br>
| Command             | Description |
| --------            | -------     |
| `make`              | Build production binary |
| `make dev`	        | Build development binary|
| `make devf`         | Build development binary, print formatted error messages |
| `make configs`      | Create all sample runs, debugging configuration files|
| `make run_dev_gdb`  | Run development binary with sample gdb binary |
| `make run_dev_pdb`  | Run development binary with sample pdb script |
| `make todo`         | Print source code tags  (`TODO`, `FIXME`, etc.) |
| `make tui_gdb`      | Start tmux GDB TUI debugging session  (see `scripts/gdb_debug`) |
| `make tui_pdb`      | Start tmux GDB TUI debugging session  (see `scripts/pdb_debug`) |
|	`make server`       | Start termfu_dev with gdbserver  (see `scripts/gdb_server`) |
| `make target`       | Start termfu_dev and target the gdbserver started  by `make server` |
<br>

`./tests/Makefile`
<br>
| Command           | Description |
| --------          | -------     |
| `make t=test1.c`  | Build, run `test1.c` |
| `make debug`      | Debug most recent build |

<br><br>

