
filetype plugin on
set tags=.tags
set tags+=$HOME/.tags_linux

command! GenCtags silent execute '!ctags -f .tags --recurse=yes --languages=C,C++ --kinds-all=* --exclude=*.c --exclude=*.cpp src' | redraw!

