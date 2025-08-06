
# GDB/MI

```
GDB/MI first line character flags
------------
~ -> d->cli_buffer     - GDB CLI console window output
^ -> d->data_buffer    - Data values e.g. source file path, line number, breakpoints
* -> d->async_buffer   - Async state change info e.g. started, stopped
  -> d->program_buffer - Debugged program's CLI output
```


## Program name, function, source file, full source name, line

### Command
```
-thread-info
```

### Data
```
debugger->data_buf
```

### No threads
```
done,
threads=[]
```

### Single thread
```
done,
threads=[
  {
    id="1",
    target-id="Thread 0x7ffff7f8d740 (LWP 14169)",
    name="hello",
    frame={
      level="0",addr="0x0000555555555195",
      func="main",
      args=[],
      file="test_programs/hello.c",
      fullname="/home/user/Code/termfu/tests/test_programs/hello.c",
      line="12",
      arch="i386:x86-64"
    },
    state="stopped",
    core="4"
  }
],
current-thread-id="1"
```

### Multiple threads
```
done,
threads=[
  {
    id="1",
    target-id="Thread 0x7ffff7f8d740 (LWP 15526)",
    name="threads",
    frame={
      level="0",
      addr="0x0000555555555344",
      func="main",
      args=[],
      file="test_programs/threads.c",
      fullname="/home/user/Code/termfu/tests/test_programs/threads.c",
      line="33",
      arch="i386:x86-64"
    },
    state="stopped",
    core="3"
  },
  {
    id="2",
    target-id="Thread 0x7ffff7bff640 (LWP 15529)",
    name="threads",
    frame={
      level="0",
      addr="0x00007ffff7ce57f8",
      func="clock_nanosleep",
      args=[],
      from="/lib/x86_64-linux-gnu/libc.so.6",
      arch="i386:x86-64"
    },
    state="stopped",
    core="6"
  },
  {
    id="3",
    target-id="Thread 0x7ffff73fe640 (LWP 15530)",
    name="threads",
    frame={
      level="0",
      addr="0x00007ffff7ce57f8",
      func="clock_nanosleep",
      args=[],
      from="/lib/x86_64-linux-gnu/libc.so.6",
      arch="i386:x86-64"
    },
    state="stopped",
    core="7"
  }
],
current-thread-id="1"
```


## Breakpoints

### Data
```
debugger->data_buf
```

### Command
```
-break-info
```

### No breakpoints
```
done,
BreakpointTable={
    nr_rows="0",
    nr_cols="6",
    hdr=[
        {width="7",alignment="-1",col_name="number",colhdr="Num"},
        {width="14",alignment="-1",col_name="type",colhdr="Type"},
        {width="4",alignment="-1",col_name="disp",colhdr="Disp"},
        {width="3",alignment="-1",col_name="enabled",colhdr="Enb"},
        {width="10",alignment="-1",col_name="addr",colhdr="Address"},
        {width="40",alignment="2",col_name="what",colhdr="What"}
    ],
    body=[]
}
```

### Single breakpoint
```
done,
BreakpointTable={
  nr_rows="1",
  nr_cols="6",
  hdr=[ ... ],
  body=[
    bkpt={
      number="1",
      type="breakpoint",
      disp="keep",
      enabled="y",
      addr="0x00000000000011f6",
      func="main",
      file="hello.c",
      fullname="/home/user/Code/termfu/test_programs/hello.c",
      line="15",
      thread-groups=["i1"],
      times="0",
      original-location="hello.c:15"
    }
  ]
}
```

### Multiple breakpoints
```
done,
BreakpointTable={
    nr_rows="2",
    nr_cols="6",
    hdr=[ ... ],
    body=[
        bkpt={
            number="1",
            type="breakpoint",
            disp="keep",
            enabled="y",
            addr="0x0000000000001195",
            func="main",
            file="hello.c",
            fullname="/home/user/Code/termfu/test_programs/hello.c",
            line="6",
            thread-groups=["i1"],
            times="0",
            original-location="main"
        },
        bkpt={
            number="2",
            type="breakpoint",
            disp="keep",
            enabled="y",
            addr="0x00000000000011f6",
            func="main",
            file="hello.c",
            fullname="/home/user/Code/termfu/test_programs/hello.c",
            line="15",
            thread-groups=["i1"],
            times="0",
            original-location="hello.c:15"
        }
    ]
}
```


## Watchpoints

### Data
```
debugger->data_buf
```

### Command
```
-data-evaluate-expression n1
```

### Variable doesn't exist
```
error,msg="No symbol \"n3\" in current context."
```

### Variable exists
```
done,value="13"
```

### Array variable out of scope
```
done,value="{0, 0, 0, 0, 0}"
```

### String variable out of scope
```
done,value="0x0"
```

### String variable with raw quotes inside string
```
done,value="0x555555556018 \"This is a long line of \\\"characters\\\" that might not fit in the window\""
```

## Local Variables

### Command
```
-stack-list-locals 1
```

### Multiple different types
```
done,
locals=[
  {
    name="n1",
    value="13"
  },
  {
    name="n_arr",
    value="{1, 2, 3, 4, 5}"
  },
  {
    name="n_len",
    value="5"
  },
  {
    name="str1",
    value="0x555555556008 \"Hello, \""
  },
  {
    name="str2",
    value="0x555555556010 \"world!\\n\""
  },
  {
    name="str3",
    value="0x555555556018 \"This is a long line of \\\"characters\\\" that might not fit in the window\""
  },
  {
    name="ds",
    value="{num = 13, ch = 97 'a', str = 0x555555556018 \"This is a long line of \\\"characters\\\" that might not fit in the window\"}"
  }
]
```
