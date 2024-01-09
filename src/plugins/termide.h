
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
int empty_func      (int li, layouts_t *layouts);
    //
int termide_back    (int li, layouts_t *layouts);
int termide_layouts (int li, layouts_t *layouts);
int termide_builds  (int li, layouts_t *layouts);



#endif
