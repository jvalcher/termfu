
#ifndef plugins_h
#define plugins_h


/*
    Builds  (build_t)
    ----
    Shell scripts defined in CONFIG_FILE for building your software 
  
    Assigned number in build array in order listed in CONFIG_FILE (0 - MAX_BUILDS-1)
  
    The current build choice, which defaults to the first in your CONFIG_FILE,
    can be called at the start of a plugin action below, as can be seen in 
    GDB's "Run" plugin.  (plugins/gdb_run.c)
  
        build  [0]  == 0
        title  [0]  == "dev_build_1"
        script [0]  == "#!/bin/bash ..."
    
        [ build : dev_build_1 ]
        #!/bin/bash
        time=$(date -u +"%Y-%m-%d %H:%M:%S UTC")
        output=$(make dev1 2>&1)
        tmp_file=$(mktemp)
        printf "$time: make dev1\n" >> "$tmp_file"
        printf "___________\n"      >> "$tmp_file"
        printf "$output\n\n\n"      >> "$tmp_file"
        cat build.out >> "$tmp_file"
        mv "$tmp_file" build.out
  
        [ build : dev_build_2 ]  
        ...
*/
#define MAX_BUILDS  20



#endif
