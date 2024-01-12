
#ifndef TERMIDE_H
#define TERMIDE_H

/*
termIDE
-------

    empty_func          plugin_function[] placeholder, not called

    termide_back        Bak     Back  (i.e. unfocus window, quit termIDE)
    termide_layouts     Lay     Display layouts
    termide_builds      Bld     Display builds
*/
int empty_func      (layout_t *layout);
    //
int termide_back    (layout_t *layout);
int termide_layouts (layout_t *layout);
int termide_builds  (layout_t *layout);



#endif
